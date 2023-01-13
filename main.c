#include <SL_DEF.h>
#include <SEGA_INT.H>
#include <SEGA_GFS.H>

#include "def.h"
#include "mymath.h"
#include "render.h"
#include "input.h"
#include "tga.h"
#include "vdp2.h"
#include "collision.h"
#include "pcmsys.h"

//
// Game data //
//Be very careful with uninitialized pointers. [In other words, INITIALIZE POINTERS!]
//

extern Sint8 SynchConst; //SGL System Variable
int framerate;
int game_set_res = TV_320x240;

unsigned char HWRAM_heap[HWRAM_MODEL_DATA_HEAP_SIZE];

void * currentAddress = (void*)LWRAM;

unsigned char * dirty_buf = (unsigned char*)LWRAM;

short close_snd;
short btn1_snd;
short btn2_snd;
short click_snd;

void	update_gamespeed(void)
{
	//Just counts vblanks between frames.
	//You want 2. Only 2. 3 is too slow, 1 is too fast, 4 is just terrible, uninstall, come back tomorrow.
	spr_sprintf(TV_WIDTH - (11 * sprAsciiWidth), 32, "Blanks:(%i)", framerate);
	framerate = 0;
}

void	init_lwram(void)
{
	//Initializes pointers to LWRAM heaps
	currentAddress = (void*)LWRAM; //Initialize loading pointer
	// Set address of scratch buffer (placed at end of LWRAM)
	dirty_buf = (void*)(LWRAM + (1042 * 1024) - 65536); 
	//In LWRAM because ... i can // 2048 bytes
	pcoTexDefs = (void*)((unsigned int)(dirty_buf-(sizeof(paletteCode) * 1024)));//|UNCACHE); 

//I have detected a 'black spot' in LWRAM, between 256 and 512KB from the end of LWRAM is what seems to be an illegal area.
}

//Draws an "Axis" at a position. Can represent an empty / entity-less object (like a light source).
void	draw_empty_object(FIXED * pos, ANGLE * rot, FIXED * size)
{
	///////////////////////////////////////
	//Drawing axis to represent an empty
	//Step 1: Push the matrix pointer
	//Step 2: Translate the matrix to the empty's world position
	// Information: Translation is negative because player is translated forward, this is translated back (negative) to where it is
	//Step 3: Draw Processing
	//Step 4: Pop the matrix pointer 
	//////////////////////////////////////
			slPushMatrix();
			slTranslate(-pos[X], -pos[Y], -pos[Z]);
			slRotY(rot[Y]);
			slRotX(rot[X]);
			slRotZ(rot[Z]);
			drawAxis(size);
			slPopMatrix();
}

////////////////////////////////////////////////////
// sunLight: Pointed to by a dynamic light as its mono directional, unit brightness lamp.
// Basically, if you set this at {0, -1<<16, 0}, light will appear to come from directly above.
FIXED sunLight[3] = {0, 0, 0};

//Parameters for drawing an axis to represent light's position
POINT lightAxisSize = {3<<16, 3<<16, 3<<16};
ANGLE lightAxisRot[XYZ] = {0,0,0};
//Parameter for drawing an axis to represent the view rotation
POINT viewAxisSize = {32768, 32768, 32768};
// Menu control setting
short controlSwitch = 0;
// Collision on/off setting
Bool collide_or_not = 0;

int pl_spd = 0;
ANGLE pl_rot[XYZ];
ANGLE ent_rot[XYZ];

entity_t drawn_entity;

void	reset(void)
{
	
	pl_rot[X] = 0;		pl_rot[Y] = 0;		pl_rot[Z] = 0;
	ent_rot[X] = 0;		ent_rot[Y] = 0;		ent_rot[Z] = 0;
	sunLight[X] = 0;	sunLight[Y] = 0;	sunLight[Z] = 0;
	
	pl_spd = 0;
	initBoxes();
	pl_RBB.pos[X] = 0;
	pl_RBB.pos[Y] = 0;
	pl_RBB.pos[Z] = (25<<16);
	controlSwitch = 0;
	collide_or_not = 0;
	
	active_lights[0].ambient_light = &sunLight[0];
	active_lights[0].min_bright = 30000;
	active_lights[0].bright = 768;
	active_lights[0].pos[X] = 30<<15;
	active_lights[0].pos[Y] = 4<<16;
	active_lights[0].pos[Z] = 30<<15;
	active_lights[0].pop = 1;
}

void	example_collision_handler(void)
{
	short normal_hit = test_collide_boxes(&RBBs[0], &pl_RBB);
	if(normal_hit >= 0)
	{
	spr_sprintf(128, 32, "Collided(%i)", normal_hit);
		pl_RBB.pos[X] = pl_RBB.prevPos[X];
		pl_RBB.pos[Y] = pl_RBB.prevPos[Y];
		pl_RBB.pos[Z] = pl_RBB.prevPos[Z];
		pl_rot[X] = pl_RBB.rot[X];
		pl_rot[Y] = pl_RBB.rot[Y];
		pl_rot[Z] = pl_RBB.rot[Z];
		
		RBBs[0].pos[X] = RBBs[0].prevPos[X];
		RBBs[0].pos[Y] = RBBs[0].prevPos[Y];
		RBBs[0].pos[Z] = RBBs[0].prevPos[Z];
		ent_rot[X] = RBBs[0].rot[X];
		ent_rot[Y] = RBBs[0].rot[Y];
		ent_rot[Z] = RBBs[0].rot[Z];
	} else {
	spr_sprintf(128, 32, "No collision");
	}
	
}

void	makeBoxes(void)
{
	if(collide_or_not) example_collision_handler();

	bound_box_starter.x_location = RBBs[0].pos[X];
	bound_box_starter.y_location = RBBs[0].pos[Y];
	bound_box_starter.z_location = RBBs[0].pos[Z];
	bound_box_starter.x_rotation = ent_rot[X];
	bound_box_starter.y_rotation = ent_rot[Y];
	bound_box_starter.z_rotation = ent_rot[Z];
	bound_box_starter.x_radius = drawn_entity.radius[X]<<16;
	bound_box_starter.y_radius = drawn_entity.radius[Y]<<16;
	bound_box_starter.z_radius = drawn_entity.radius[Z]<<16;
	bound_box_starter.modified_box = &RBBs[0];
	make2AxisBox(&bound_box_starter);
	
	bound_box_starter.x_location = pl_RBB.pos[X];
	bound_box_starter.y_location = pl_RBB.pos[Y];
	bound_box_starter.z_location = pl_RBB.pos[Z];
	bound_box_starter.x_rotation = pl_rot[X];
	bound_box_starter.y_rotation = pl_rot[Y];
	bound_box_starter.z_rotation = pl_rot[Z];
	bound_box_starter.x_radius = 5<<16;
	bound_box_starter.y_radius = 5<<16;
	bound_box_starter.z_radius = 5<<16;
	bound_box_starter.modified_box = &pl_RBB;
	make2AxisBox(&bound_box_starter);
}

void	control_menu_system(void)
{
	
	static short menuToggle = 0;
	static VECTOR controlUV;
	static int apply_color_offset = 1;
	static int display_control_preview = 1;
	
	static __basic_menu mnu;
	
	if(is_key_struck(DIGI_START))
	{
		if(menuToggle){
			pcm_play(close_snd, PCM_SEMI, 7);
			menuToggle = 0;
		} else {
			pcm_play(btn1_snd, PCM_SEMI, 7);
			menuToggle = 1;
		}
		mnu.selection = 0;
		restore_vdp1_palette();
		apply_color_offset = 1;
	}
	
	if(!menuToggle)
	{
		nbg_sprintf(slLocate(8,0), "Great Value Render Demo");
		nbg_sprintf(slLocate(8,1), "Press START for Menu");
		setFramebufferEraseRegion(0, 0, TV_WIDTH, TV_HEIGHT);
		drawn_entity.useClip = 1; //Clip inside default clip area (screen)
		
		if(controlSwitch < 2 && display_control_preview)
		{
			nbg_sprintf(slLocate(1,25), "D-Pad: Rotation");
			nbg_sprintf(slLocate(1,24), "B/Y: Move fwd/bck");
		}
		
		if(controlSwitch == 0)
		{
			nbg_sprintf(slLocate(1,26),"Controlling Player    ");
			if(is_key_down(DIGI_UP))	pl_rot[X]-=182;
			if(is_key_down(DIGI_DOWN))	pl_rot[X]+=182;
												  
			if(is_key_down(DIGI_RIGHT))	pl_rot[Y]-=182;
			if(is_key_down(DIGI_LEFT))	pl_rot[Y]+=182;
			
			if(is_key_down(DIGI_B))	{
				pl_spd = 5;
			} else if(is_key_down(DIGI_Y))	{
				pl_spd = -5;
			} else {
				pl_spd = 0;
			}
			
			pl_RBB.pos[X] += fxm(pl_spd<<12, slCos(pl_RBB.rot[Y] + (90 * 182)));
			pl_RBB.pos[Z] += fxm(pl_spd<<12, slSin(pl_RBB.rot[Y] + (90 * 182)));
		} else if(controlSwitch == 1)
		{
			nbg_sprintf(slLocate(1,26),"Controlling Entity   ");
			if(is_key_down(DIGI_UP))	ent_rot[X]-=182;
			if(is_key_down(DIGI_DOWN))	ent_rot[X]+=182;
										
			if(is_key_down(DIGI_RIGHT))	ent_rot[Y]-=182;
			if(is_key_down(DIGI_LEFT))	ent_rot[Y]+=182;
			//Control Vector Lineator
			controlUV[X] = -(fxm(slSin(RBBs[0].rot[Y]),slCos(RBBs[0].rot[X])));
			controlUV[Y] = -(slSin(RBBs[0].rot[X]) );
			controlUV[Z] = -(fxm(slCos(RBBs[0].rot[Y]),slCos(RBBs[0].rot[X])));
			
			if(is_key_down(DIGI_Y))
			{
				RBBs[0].pos[X] += controlUV[X]>>1;
				RBBs[0].pos[Y] += controlUV[Y]>>1;
				RBBs[0].pos[Z] += controlUV[Z]>>1;
			}
			if(is_key_down(DIGI_B))
			{
				RBBs[0].pos[X] -= controlUV[X]>>1;
				RBBs[0].pos[Y] -= controlUV[Y]>>1;
				RBBs[0].pos[Z] -= controlUV[Z]>>1;
			}
			
		} else if(controlSwitch == 2)
		{
			
			if(display_control_preview)
			{
			nbg_sprintf(slLocate(1,25), "D-Pad: X/Z Angle");
			nbg_sprintf(slLocate(1,24), "B/Y: Y Angle");
			}
			nbg_sprintf(slLocate(1,26),"Controlling Sun Angle");

			if(is_key_down(DIGI_RIGHT))	sunLight[Z]-=182;
			if(is_key_down(DIGI_LEFT))	sunLight[Z]+=182;

			if(is_key_down(DIGI_UP))	sunLight[X]-=182;
			if(is_key_down(DIGI_DOWN))	sunLight[X]+=182;
			
			if(is_key_down(DIGI_Y))		sunLight[Y]-=182;
			if(is_key_down(DIGI_B))		sunLight[Y]+=182;
		}
		
	} else {
		
		color_offset_vdp1_palette(-(0x1F1F1F), &apply_color_offset);
		
		nbg_sprintf(slLocate(8,0), "                       ");
		nbg_sprintf(slLocate(8,1), "                       ");

		mnu.topLeft[X] = 0;
		mnu.topLeft[Y] = 0;
		mnu.scale[X] = 80;
		mnu.scale[Y] = 32;
		mnu.option_grid[X] = 4;
		mnu.option_grid[Y] = 2;
		mnu.num_option = 8;
		mnu.backColor = 16;
		mnu.optionColor = 5;
		char * option_list[] = {"Ctrl Plyr", "Ctrl Ent", "Snap Lite", "Toggle Lite", "Ctrl Sun", "Reset", "Collide Tgl", "Ctrl Tgl"};
		mnu.option_text = option_list;

		menu_with_options(&mnu);
		setUserClippingAtDepth(mnu.topLeft, mnu.btmRight, 999<<16);
		drawn_entity.useClip = 2; //Clip outside this area
		
			if(is_key_struck(DIGI_RIGHT))	mnu.selection++;
			if(is_key_struck(DIGI_LEFT))	mnu.selection--;
			if(is_key_struck(DIGI_DOWN))	mnu.selection+= (mnu.option_grid[X]);
			if(is_key_struck(DIGI_UP))		mnu.selection-= (mnu.option_grid[X]);
			if(is_key_struck(DIGI_RIGHT) | is_key_struck(DIGI_LEFT)
				| is_key_struck(DIGI_DOWN) | is_key_struck(DIGI_UP)) pcm_play(btn2_snd, PCM_SEMI, 7);

		if(is_key_struck(DIGI_A))
		{
			pcm_play(click_snd, PCM_SEMI, 7);
			switch(mnu.selection)
			{
				case(0):
				controlSwitch = 0;
				nbg_sprintf(slLocate(1,26),"Controlling Player   ");
				break;
				case(1):
				controlSwitch = 1;
				nbg_sprintf(slLocate(1,26),"Controlling Entity   ");
				break;
				case(2):
				active_lights[0].pos[X] = pl_RBB.pos[X];
				active_lights[0].pos[Z] = pl_RBB.pos[Z];
				nbg_sprintf(slLocate(1,26),"Light Snap to Player ");
				break;
				case(3):
				nbg_sprintf(slLocate(1,26),"Shine/Shade Toggle   ");
				active_lights[0].bright *= -1;
				break;
				case(4):
				nbg_sprintf(slLocate(1,26),"Controlling Sun Angle");
				controlSwitch = 2;
				break;
				case(5):
				nbg_sprintf(slLocate(1,26),"        Reset        ");
				reset();
				break;
				case(6):
				nbg_sprintf(slLocate(1,26),"Collision Toggled    ");
				collide_or_not = (collide_or_not) ? 0 : 1;
				break;
				case(7):
				if(display_control_preview)
				{
				nbg_sprintf(slLocate(1, 26), "Control Preview Off  ");
				nbg_sprintf(slLocate(1, 25), "                     ");
				nbg_sprintf(slLocate(1, 24), "                     ");
				display_control_preview = 0;
				} else {
				nbg_sprintf(slLocate(1, 26), "Control Preview On   ");
				display_control_preview = 1;
				}
				break;
				default:
				break;
			}
			
		}
	}
}

void	draw(void)
{
		
		//Draw an axis with a fixed position to represent the player's rotation
		POINT viewAxisPos = {(4<<16) + 45192, (3<<16), -NEAR_PLANE_DISTANCE - (1<<16)};
		draw_empty_object(viewAxisPos, pl_RBB.rot, viewAxisSize);
		//Draw an axis with a fixed position to represent the sun angle
		POINT sunAxisPos = {(3<<16) + 45192, (3<<16), -NEAR_PLANE_DISTANCE - (1<<16)};
		ANGLE sunAngle[XYZ] = {sunLight[X], sunLight[Y], sunLight[Z]};
		draw_empty_object(sunAxisPos, sunAngle, sunLight);
	
		slPushMatrix();
		slRotX(pl_RBB.rot[X]);
		slRotY(pl_RBB.rot[Y]);
		slTranslate(pl_RBB.pos[X], pl_RBB.pos[Y], pl_RBB.pos[Z]);
		
		slPushMatrix();
			drawn_entity.prematrix = (FIXED*)&RBBs[0];
			ssh2DrawModel(&drawn_entity);
		slPopMatrix();
		
		draw_empty_object(active_lights[0].pos, lightAxisRot, lightAxisSize);

		slPopMatrix();

}

		MATRIX msh2Matrix; // To be used for drawing with master SH2

void	running_frame(void)
{
	
	reset();
	
	do{
		frame_render_prep();
		
		update_gamespeed();
		slSlaveFunc(draw,0);
		slCashPurge();
		makeBoxes();

		control_menu_system();

	/////////////////////////////////
	/*
	Information:
	In SGL, only the slave SH2 can sort the polygon buffer, as it is the CPU who manages preparing the command list.
	Because of this, draw commands sent by the SH2 are placed into the buffer without sorting. 
	The slave SH2 must come back and sort them later.
	This is important to note: you must make sure that this command is issued to the SSH2 after all MSH2 draw calls are in.
	In this case, this includes the drawing of text to the screen via VDP1.
	This is also not a binary rule. The SSH2 can freely insert commands into the buffer, and sort them later if it wishes.
	However, you really do want to trigger this function as soon as you know MSH2 is done drawing.
	Here, it is just at the end of the frame right before slSynch. This may not always be an ideal place for it.
	
	As for the sending of draw stats, "0" sends none, "1" sends via sprite text, "2" sends via VDP2 NBG text.
	*/
	////////////////////////////////
		slSlaveFunc(sort_master_polys, 0);
		reset_pad(&pad1);
		slSynch();
	} while(1);
}

void	blank_out(void){
	vblank_requirements();
	operate_digital_pad1();
	sdrv_vblank_rq();
	framerate++;
}


int	main(void)
{

	//jo_core_init(0x9CE7); // Color is the value
	game_set_res = (hi_res_switch) ? TV_704x448 : TV_352x224;
	slInitSystem(game_set_res, NULL, 2);
	slDynamicFrame(ON); //Dynamic framerate
    SynchConst = (Sint8)2;  //Framerate control. 1/60 = 60 FPS, 2/60 = 30 FPS, etc.
	if(hi_res_switch){
	slZoomNbg0(32768, 32768);
	}
	init_lwram();
	init_render_area(90 * 182);
	init_vdp2(0x9986);
	cd_init();
	
	load_drv(ADX_MASTER_2304);
	//Set the address to the model data heap in high work RAM
	/////////////////////////////////
	/*
	Information about model data location:
	You can put model data in low work RAM or high work RAM.
	However, this linearly affects the speed on the CPU. It does not affect raster speed on VDP1; it already has the data.
	Rendering from LWRAM takes about twice as long as rendering from HWRAM. So if you can, you should use HWRAM.
	The amount of data you have available for 3D models in HWRAM fluctuates highly depending on your code.
	Remember, your code is in RAM too. It takes up space, any other static allocation you make will be in HWRAM too.
	*/
	/////////////////////////////////
	currentAddress = &HWRAM_heap[0];
	
	WRAP_NewPalette((Sint8*)"TADA.TGA", dirty_buf);
	baseAsciiTexno = numTex;
	sprAsciiHeight = 12;
	sprAsciiWidth = WRAP_NewTable((Sint8*)"FONT.TGA", dirty_buf, sprAsciiHeight); //last argument, tex height
	
	currentAddress = gvLoad3Dmodel((Sint8*)"SPHER.GVP", currentAddress, &drawn_entity, GV_SORT_CEN, 'N');
	
	close_snd = load_8bit_pcm((Sint8*)"CLOSE.PCM", 23040);
	btn1_snd = load_8bit_pcm((Sint8*)"BUTTON1.PCM", 23040);
	btn2_snd = load_8bit_pcm((Sint8*)"BUTTON2.PCM", 23040);
	click_snd = load_8bit_pcm((Sint8*)"CLICK.PCM", 23040);
	
	//anim_defs();
	
	//
	slIntFunction(blank_out);
	initBoxes();
	//

	running_frame();
	return 1;
}

