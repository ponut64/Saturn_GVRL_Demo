
#include <sl_def.h>
#include <SEGA_GFS.H>
#include "tga.h"

#include "bounder.h"

#include "mloader.h"

entity_t entities[MAX_MODELS];

/**
Modified by ponut for madness
**/
unsigned char setTextures(entity_t * model, short baseTexture)
{
	gvAtr smpAttr;
	short maxTex = 0;
	
	model->base_texture = baseTexture;
	
	for(unsigned int i = 0; i < model->pol->nbPolygon; i++)
	{
		smpAttr = model->pol->attbl[i];
		maxTex = (maxTex < smpAttr.texno) ? smpAttr.texno : maxTex;
	}
	
	for(unsigned int i = 0; i < model->pol->nbPolygon; i++)
	{
		smpAttr = model->pol->attbl[i];
		
		model->pol->attbl[i].texno += baseTexture;
	}

	return maxTex;
}

//Gets texture information from small headers, and sends texture data to VRAM.
void * loadTextures(void * workAddress, entity_t * model)
{
	
	//unsigned short * debug_addr = (unsigned short *)workAddress;
	unsigned char * readByte = (unsigned char *)workAddress;
	unsigned char tHeight = 0;
	unsigned char tWidth = 0;
	unsigned int tSize = 0;
	// jo_printf(0, 14, "(%i)", model->numTexture);
	// jo_printf(0, 15, "(%i)", debug_addr[0]);
	for(int j = 0; j < model->numTexture+1; j++)
	{
		readByte+=2;	//Skip over a boundary short word, 0xF7F7
		tHeight = readByte[0];
		tWidth = readByte[1];
		tSize = tHeight * tWidth;
		readByte += 2; //Skip over the H x W bytes
		GLOBAL_img_addr = readByte;
		add_texture_to_vram((unsigned short)tHeight, (unsigned short)tWidth);
		readByte += tSize;
	}
	return (void*)readByte;
}

void * loadAnimations(void * startAddress, entity_t * model, modelData_t * modelData)
{
    void * workAddress = startAddress;
    unsigned int a; //, ii;

    for (a=0; a<modelData->nbFrames; a++) 
    {
        model->animation[a]=(anim_struct*)(workAddress);
        workAddress=(void*)(workAddress+sizeof(anim_struct));

        unsigned int totPoints=0;
        unsigned int totNormals=0;


            totPoints += model->pol->nbPoint;
            totNormals += model->pol->nbPolygon;

        {

            model->animation[a]->cVert = (compVert*)(workAddress);
            workAddress=(void*)(workAddress+(sizeof(compVert) * totPoints));

            if (totPoints % 2 != 0){
               workAddress=(void*)(workAddress+(sizeof(short)));
			}
            model->animation[a]->cNorm = (compNorm*)(workAddress);
            workAddress=(void*)(workAddress+(sizeof(compNorm) * totNormals));
            while (totNormals % 4 != 0)
            {
                workAddress=(void*)(workAddress+(sizeof(char)));
                totNormals++;
            }

        }
    }

    return workAddress;

}

void * loadPDATA(void * startAddress, entity_t * model)
{
    void * workAddress = startAddress;

        model->pol=(GVPLY*)workAddress;
        workAddress=(void*)(workAddress + sizeof(GVPLY));
        model->pol->pntbl = (POINT*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(POINT) * model->pol->nbPoint));
        model->pol->pltbl = (POLYGON*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(POLYGON) * model->pol->nbPolygon));
        model->pol->attbl = (gvAtr*)workAddress;
        workAddress=(void*)(workAddress + (sizeof(gvAtr) * model->pol->nbPolygon));
   

    return workAddress;
}

void * gvLoad3Dmodel(Sint8 * filename, void * startAddress, entity_t * model, unsigned short sortType, char modelType)
{

	modelData_t * model_header;
	void * workAddress = startAddress;
	model->type = modelType;
	GfsHn gfs_mdat;
	Sint32 sector_count;
	Sint32 file_size;
	
	Sint32 local_name = GFS_NameToId(filename);

//Open GFS
	gfs_mdat = GFS_Open((Sint32)local_name);
//Get sectors
	GFS_GetFileSize(gfs_mdat, NULL, &sector_count, NULL);
	GFS_GetFileInfo(gfs_mdat, NULL, NULL, &file_size, NULL);
	
	GFS_Close(gfs_mdat);
	
	GFS_Load(local_name, 0, (Uint32 *)workAddress, file_size);
	
	GFS_Close(gfs_mdat);
	
	// slDMACopy(workAddress, &model_header, sizeof(modelData_t));
	model_header = (modelData_t *)workAddress;

	model->first_portal = (unsigned char)model_header->first_portal;

	//ADDED
    model->nbMeshes = model_header->TOTAL_MESH;
	model->nbFrames = model_header->nbFrames;
	
	Sint32 bytesOff = (sizeof(modelData_t)); 
	workAddress = (workAddress + bytesOff); //Add the binary meta data size to the work address to reach the PDATA
	
	model->size = (unsigned int)workAddress;
	workAddress = loadPDATA((workAddress), model);
	model->size = (unsigned int)workAddress - model->size;

	model->numTexture = setTextures(model, numTex); //numTex is a tga.c directive

    workAddress = loadAnimations(workAddress, model, model_header);
	// A temporary address is used to retrieve the following data.
	// This is used because the following data is to be overwritten whenever new model data is loaded.
	// To facilitate this, workAddress is pointed forward past all important data that must not be overwritten.
	// Thus the temporary pointer is pointing to all data that can be thrown out once parsed.
	//void * temporaryAddress;
	/*temporaryAddress = */loadTextures(workAddress, model);
	//unsigned char * readByte = temporaryAddress;
	////////////////
	// If the model type is 'B' (for BUILDING), create combined textures.
	// Also read the item data at the end of the payload.
	////////////////
	if(model->type == 'B')
	{
		for(int j = 0; j < model->numTexture+1; j++)
		{
			make_combined_textures(model->base_texture + j);
		}
		/////////////////////////////////////////////
		// Item Data Payload
		// It is appended at the end of the binary, past the textures.
		// It is copied out of this region for permanent use in the BuildingPayload struct.
		// It's order is:
		// 0 byte: total items
		// 1 byte: unique items
		// every 8 bytes after
		// item number, x, y, z, position (relative to entity) as 16-bit int
		//
		// The converter tool can output this data, but this loader is not equipped for it, because it is a game engine feature.
		//
		/////////////////////////////////////////////
	} 

	
	//////////////////////////////////////////////////////////////////////
	// Set radius
	//////////////////////////////////////////////////////////////////////
	model->radius[X] = model_header->radius[X];
	model->radius[Z] = model_header->radius[Z];
	model->radius[Y] = model_header->radius[Y];
	//NOTE: We do NOT add the size of textures to the work address pointer.
	//The textures are at the end of the GVP payload and have no need to stay in work RAM. They are in VRAM.
	
	// jo_printf(0, 9, "(%i)H", tHeight);
	// jo_printf(0, 10, "(%i)W", tWidth);
	// jo_printf(0, 11, "(%i)T", tSize);
		if(sortType != 0)
		{
	for(unsigned int i = 0; i < model->pol->nbPolygon; i++)
	{
		//Decimate existing sort type bits
	model->pol->attbl[0].render_data_flags &= 207;
		//Inject new sort type bits
	model->pol->attbl[0].render_data_flags |= sortType;
		//New render path only reads first attbl for sorting
	}
		}
	
	model->file_done = 1;
	
	//Alignment
	volatile unsigned int aligning_address = (volatile unsigned int)workAddress;
	aligning_address += 4;
	aligning_address &= 0xFFFFFFFC;
	workAddress = (void*)aligning_address;
	
	return workAddress;
}

void	init_entity_list(void)
{
	int bytes_to_clear = sizeof(entity_t) * MAX_MODELS;
	unsigned char * byte_pointer = (void*)&entities[0];
	
	for(int i = 0; i < bytes_to_clear; i++)
	{
		byte_pointer[i] = 0;
	}
	
}

