/*
UNDERWORLD EXPORTER
D3DarkMod.cpp

Functions for writing out the .map files

*/
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "math.h"
#include "tilemap.h"
#include "main.h"
#include "textures.h"
#include "gameobjects.h"
#include "gameobjectsrender.h"
#include "D3DarkMod.h"
#include "D3DarkModTiles.h"

//The length, breadth and height of a tile unit. BrushSizeZ is a single step.
int BrushSizeX;
int BrushSizeY;
int BrushSizeZ;

int PrimitiveCount;	//Counter for the worldspawn primities.
int EntityCount;	//Counter for the object entities.
int CEILING_HEIGHT;
tile LevelInfo[64][64];
int iGame;

void CalcObjectXYZ(int game, float *offX, float *offY, float *offZ, tile LevelInfo[64][64], ObjectItem objList[1600], long nextObj, int x, int y);
float calcAlignmentFactor(float adjacent, float opposite);
void AddEmails(int game, tile LevelInfo[64][64], ObjectItem objList[1600]);
void RenderShockDoorway(int game, int x, int y, tile &t, ObjectItem currDoor, tile LevelInfo[64][64], ObjectItem objList[1600]);
float getSteepOffset(int steepness);

int levelNo;
long SHOCK_CEILING_HEIGHT;
long UW_CEILING_HEIGHT;

extern FILE *MAPFILE;

void RenderDarkModLevel(tile LevelInfo[64][64], ObjectItem objList[1600], int game)
	{
	/*Main processing loop for generating the level to the DarkMod/IDTech4 format.
	Runs through the tilemap array and calls the relevant tile rendering code.
	*/

	int x;
	int y;

	iGame = game;

	//Levels use different ceiling heights.
	//Shock is variable depending on tile type, UW is fixed.
	//In shock the ceiling height of tiles are calculated from the top.
	switch (game)
		{
			case SHOCK:
				{
				CEILING_HEIGHT = SHOCK_CEILING_HEIGHT;
				break;
				}
			default://UW1&2
				{
				CEILING_HEIGHT = UW_CEILING_HEIGHT;
				break;
				}
		}

	//File header of the map file.
	fprintf(MAPFILE, "Version 2\n");
	fprintf(MAPFILE, "// entity 0\n{\n\"classname\" \"worldspawn\"\n");
	//sick of starting at origin in dr.
	fprintf(MAPFILE, "\"editor_drLastCameraPos\" \"2594.79 -1375.88 1780.4\"\n");
	fprintf(MAPFILE, "\"editor_drLastCameraAngle\" \"-28.5 90.9 0\"\n");
	PrimitiveCount = 0;
	fprintf(MAPFILE, "\n");
	//For D3 mode I take several passes for floors, walls and ceilings. Each time I redo the cleanup of tiles so as the reduce the number of primitives.

	//Cleanup solids. Hidden solids are removed.
	ResetCleanup(LevelInfo, game);
	CleanUpHiddenTiles(LevelInfo, game);
	CleanUp(LevelInfo, game, CLEANUPXAXIS, TILE_SOLID, SURFACE_FLOOR);
	CleanUp(LevelInfo, game, CLEANUPYAXIS, TILE_SOLID, SURFACE_FLOOR);
	for (y = 0; y <= 63; y++)
		{
		for (x = 0; x <= 63; x++)
			{
			if ((LevelInfo[x][y].hasElevator == 0) && (LevelInfo[x][y].TerrainChange == 0) && (LevelInfo[x][y].BullFrog == 0) && (LevelInfo[x][y].tileType == TILE_SOLID))
				{
				RenderDarkModTile(game, x, y, LevelInfo[x][y], 0, 0, 0, 0);//Render my solid tiles
				}
			}
		}
	if (game == SHOCK)
		{
		//Shock ceilings are rendered seperately.
		//Cleanup Ceilings
		ResetCleanup(LevelInfo, game);
		CaulkHiddenWalls(LevelInfo, game, SURFACE_CEIL);
		CleanUp(LevelInfo, game, CLEANUPXAXIS, TILE_OPEN, SURFACE_CEIL);
		CleanUp(LevelInfo, game, CLEANUPYAXIS, TILE_OPEN, SURFACE_CEIL);
		CleanUp(LevelInfo, game, CLEANUPXAXIS, TILE_SLOPE_N, SURFACE_CEIL);
		CleanUp(LevelInfo, game, CLEANUPXAXIS, TILE_SLOPE_S, SURFACE_CEIL);
		CleanUp(LevelInfo, game, CLEANUPYAXIS, TILE_SLOPE_E, SURFACE_CEIL);
		CleanUp(LevelInfo, game, CLEANUPYAXIS, TILE_SLOPE_W, SURFACE_CEIL);

		for (y = 0; y <= 63; y++)
			{
			for (x = 0; x <= 63; x++)
				{
				if ((LevelInfo[x][y].hasElevator == 0) && (LevelInfo[x][y].TerrainChange == 0) && (LevelInfo[x][y].BullFrog == 0) && (LevelInfo[x][y].tileType != TILE_SOLID))
					{
					RenderDarkModTile(game, x, y, LevelInfo[x][y], 0, 0, 1, 0);//Render my ceilings.
					}
				}
			}
		}

	//Cleanup Floors
	ResetCleanup(LevelInfo, game);
	CaulkHiddenWalls(LevelInfo, game, SURFACE_FLOOR);
	CleanUp(LevelInfo, game, CLEANUPXAXIS, TILE_OPEN, SURFACE_FLOOR);
	CleanUp(LevelInfo, game, CLEANUPYAXIS, TILE_OPEN, SURFACE_FLOOR);
	CleanUp(LevelInfo, game, CLEANUPXAXIS, TILE_SLOPE_N, SURFACE_FLOOR);
	CleanUp(LevelInfo, game, CLEANUPXAXIS, TILE_SLOPE_S, SURFACE_FLOOR);
	CleanUp(LevelInfo, game, CLEANUPYAXIS, TILE_SLOPE_E, SURFACE_FLOOR);
	CleanUp(LevelInfo, game, CLEANUPYAXIS, TILE_SLOPE_W, SURFACE_FLOOR);
	for (y = 0; y <= 63; y++)
		{
		for (x = 0; x <= 63; x++)
			{
			if ((LevelInfo[x][y].isWater == 0) && (LevelInfo[x][y].hasElevator == 0) && (LevelInfo[x][y].TerrainChange == 0) && (LevelInfo[x][y].BullFrog == 0) && (LevelInfo[x][y].tileType != TILE_SOLID))
				{
				RenderDarkModTile(game, x, y, LevelInfo[x][y], 0, 0, 0, 1);//Render my floors.
				}
			}
		}

	ResetCleanup(LevelInfo, game);

	//Special UW case for Diag tiles with water.
	for (y = 0; y <= 63; y++)
		{
		for (x = 0; x <= 63; x++)
			{
			if ((LevelInfo[x][y].isWater == 1) && (LevelInfo[x][y].hasElevator == 0) && (LevelInfo[x][y].TerrainChange == 0) && (LevelInfo[x][y].BullFrog == 0)
				&& ((LevelInfo[x][y].tileType >= TILE_DIAG_SE) && (LevelInfo[x][y].tileType <= TILE_DIAG_NW)))
				{
				RenderDarkModTile(game, x, y, LevelInfo[x][y], 0, 0, 0, 1); //The wall on diag water tiles.
				}
			}
		}

	//Render doorways
	for (y = 0; y <= 63; y++)
		{
		for (x = 0; x <= 63; x++)
			{
			if ((LevelInfo[x][y].hasElevator == 0))//Elevators are rendered as func_statics
				{
				if (LevelInfo[x][y].isDoor == 1)
					{//Adds a UW door frame.
					RenderDoorway(game, x, y, LevelInfo[x][y], objList[LevelInfo[x][y].DoorIndex]);
					}
				if (LevelInfo[x][y].shockDoor == 1)
					{//Adds a Shock door frame.
					RenderShockDoorway(game, x, y, LevelInfo[x][y], objList[LevelInfo[x][y].DoorIndex], LevelInfo, objList);
					}
				}
			}
		}
	RenderElevatorLeakProtection(game, LevelInfo);
	if (game != SHOCK)
		{//Special brushwork for UW.
		RenderFloorAndCeiling(game, LevelInfo);	//Shocks ceils are already done as part of the til.
		RenderPillars(game, LevelInfo, objList);
		}
	fprintf(MAPFILE, "}");	//End worldspawn section of the .map file.

	//Now start rendering entities.			
	EntityCount = 1;
	switch (game)
		{
			case UWDEMO:
				RenderObjectList(game, LevelInfo, objList);
				break;
			case UW1:
				RenderObjectList(game, LevelInfo, objList);
				RenderLevelExits(game, LevelInfo, objList);
				RenderChangeTerrainTiles(game, LevelInfo);
				if (levelNo == 3)
					{
					RenderBullFrogTiles(game, LevelInfo);
					}
				break;
			case UW2:
				RenderObjectList(game, LevelInfo, objList);
				RenderChangeTerrainTiles(game, LevelInfo);
				break;
			case SHOCK:
				RenderObjectList(game, LevelInfo, objList);
				AddEmails(game, LevelInfo, objList);
				for (y = 0; y <= 63; y++)
					{
					for (x = 0; x <= 63; x++)
						{
						if (LevelInfo[x][y].hasElevator >= 1)
							{//render the shock elevators
							ObjectItem tempObj;
							tempObj.tileX = x; tempObj.tileY = y;
							RenderEntityElevator(game, LevelInfo, tempObj);
							}
						}
					}
				break;
		}
	//Now render the water
	int currRegion = 1;
	for (x = 0; x<64; x++)
		{//Loop through until we find a matching region that is both water and of this id.
		for (y = 0; y<64; y++)
			{
			if ((LevelInfo[x][y].roomRegion == currRegion) && (LevelInfo[x][y].isWater == 1))
				{//Found a water region.
				fprintf(MAPFILE, "\n");
				PrimitiveCount = 0;	//resets for each entity.
				fprintf(MAPFILE, "// entity %d\n", EntityCount);
				fprintf(MAPFILE, "{\n\"classname\" \"atdm:liquid_water\"\n");
				fprintf(MAPFILE, "\n\"name\" \"WaterRegion_%02d\"\n", currRegion);
				fprintf(MAPFILE, "\n\"model\" \"WaterRegion_%02d\"\n", currRegion);
				fprintf(MAPFILE, "\n\"underwater_gui\" \"guis\\underwater\\underwater_green_thinmurk.gui\"\n");
				for (int i = 0; i < 64; i++)
					{//We've found a match so lets now render all the members of that region. we search again. Sigh.
					for (int j = 0; j < 64; j++)
						{
						if ((LevelInfo[i][j].isWater == 1) && (LevelInfo[i][j].roomRegion == currRegion))
							{
							RenderWaterTiles(game, LevelInfo, i, j);
							}
						}
					}
				fprintf(MAPFILE, "}\n");
				EntityCount++;
				currRegion++;
				x = -1; y = -1;	//restart the loop.
				}
			}
		}

	if ((game == SHOCK) && (ENABLE_LIGHTING))
		{//Light her up based on shade values. Shock currently has 2 lights per tile (sigh again)
		//This implementation of light is slightly wrong. I'm working on it. ;-)
		for (y = 0; y <= 63; y++)
			{
			for (x = 0; x <= 63; x++)
				{
				if (LevelInfo[x][y].tileType != 0)
					{
					float shade = 0.50;	//Max brightness.
					//put a light here. absolute frame rate killer. Need to merge contigous regions of light together or do something clever with texture brightness
					fprintf(MAPFILE, "// entity %d\n", EntityCount++);
					fprintf(MAPFILE, "{\n\"classname\" \"light\"");
					fprintf(MAPFILE, "\n\"name\" \"light_%02d_%02d_upper\"", x, y);
					fprintf(MAPFILE, "\n\"origin\" \"%d %d %d\"",
						x*BrushSizeX + BrushSizeX / 2,
						y*BrushSizeY + BrushSizeY / 2,
						(LevelInfo[x][y].floorHeight + (3 * (CEILING_HEIGHT - LevelInfo[x][y].ceilingHeight - LevelInfo[x][y].floorHeight) / 4))* BrushSizeZ);
					fprintf(MAPFILE, "\n\"light_center\" \"0 0 0\"");
					fprintf(MAPFILE, "\n\"light_radius\" \"%d %d %d\"",
						40 + BrushSizeX / 2,
						40 + BrushSizeY / 2,
						((CEILING_HEIGHT - LevelInfo[x][y].ceilingHeight - LevelInfo[x][y].floorHeight) / 2)*BrushSizeZ);

					shade = (0.50) * (1 - ((float)LevelInfo[x][y].shockShadeUpper / 15));
					fprintf(MAPFILE, "\n\"_color\" \"%f %f %f\"", shade, shade, shade);
					fprintf(MAPFILE, "\n\"nodiffuse\" \"0\"");
					fprintf(MAPFILE, "\n\"noshadows\" \"1\"");
					fprintf(MAPFILE, "\n\"nospecular\" \"0\"");
					fprintf(MAPFILE, "\n\"parallel\" \"0\"");
					//fprintf (MAPFILE, "\n\"texture\" \"lights/tdmnofalloff\"");
					fprintf(MAPFILE, "\n\"texture\" \"lights/square\"");
					fprintf(MAPFILE, "\n}\n");

					fprintf(MAPFILE, "// entity %d\n", EntityCount++);
					fprintf(MAPFILE, "{\n\"classname\" \"light\"");
					fprintf(MAPFILE, "\n\"name\" \"light_%02d_%02d_lower\"", x, y);
					fprintf(MAPFILE, "\n\"origin\" \"%d %d %d\"",
						x*BrushSizeX + BrushSizeX / 2,
						y*BrushSizeY + BrushSizeY / 2,
						(LevelInfo[x][y].floorHeight + (1 * (CEILING_HEIGHT - LevelInfo[x][y].ceilingHeight - LevelInfo[x][y].floorHeight) / 4))* BrushSizeZ);
					fprintf(MAPFILE, "\n\"light_center\" \"0 0 0\"");
					fprintf(MAPFILE, "\n\"light_radius\" \"%d %d %d\"",
						40 + BrushSizeX / 2,
						40 + BrushSizeY / 2,
						((CEILING_HEIGHT - LevelInfo[x][y].ceilingHeight - LevelInfo[x][y].floorHeight) / 2)*BrushSizeZ);

					shade = (0.50) * (1 - ((float)LevelInfo[x][y].shockShadeLower / 15));
					fprintf(MAPFILE, "\n\"_color\" \"%f %f %f\"", shade, shade, shade);
					fprintf(MAPFILE, "\n\"nodiffuse\" \"0\"");
					fprintf(MAPFILE, "\n\"noshadows\" \"1\"");
					fprintf(MAPFILE, "\n\"nospecular\" \"0\"");
					fprintf(MAPFILE, "\n\"parallel\" \"0\"");
					//fprintf (MAPFILE, "\n\"texture\" \"lights/tdmnofalloff\"");
					fprintf(MAPFILE, "\n\"texture\" \"lights/square\"");
					fprintf(MAPFILE, "\n}\n");

					}
				}
			}
		}
	else
		{
		//Ambient world light for when I've turned off lighting while testing.
		fprintf(MAPFILE, "// entity %d\n", EntityCount++);
		fprintf(MAPFILE, "{\n\"classname\" \"atdm:ambient_world\"");
		fprintf(MAPFILE, "\n\"name\" \"ambient_world\"", EntityCount);
		fprintf(MAPFILE, "\n\"origin\" \"%d %d 120\"", 32 * BrushSizeX, 32 * BrushSizeY);	//May cause leaks on small maps.
		fprintf(MAPFILE, "\n\"light_center\" \"0 0 0\"");
		fprintf(MAPFILE, "\n\"light_radius\" \"4500 4500 2500\"");
		fprintf(MAPFILE, "\n\"_color\" \"0.21 0.21 0.21\"");
		fprintf(MAPFILE, "\n\"nodiffuse\" \"0\"");
		fprintf(MAPFILE, "\n\"noshadows\" \"0\"");
		fprintf(MAPFILE, "\n\"nospecular\" \"0\"");
		fprintf(MAPFILE, "\n\"parallel\" \"0\"");
		fprintf(MAPFILE, "\n}\n");
		}

	switch (game)	//Some game specific stuff
		{
			case UWDEMO:
			case UW1:
				if (levelNo <= 2)
					{//Temp code for providing a player start on level one as well as a "head" light.
					fprintf(MAPFILE, "// entity %d\n", EntityCount++);
					fprintf(MAPFILE, "{\n\"classname\" \"info_player_start\"");
					fprintf(MAPFILE, "\n\"name\" \"info_player_start\"");

					switch (levelNo)
						{
							case 0:
								fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 3840.0, 250.0, 370.0); break;

							case 2:
								fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 660.0, 255.0, 370.0); break;

							default:
								fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 3840.0, 250.0, 360.0);
						}

					fprintf(MAPFILE, "\n}\n");

					fprintf(MAPFILE, "// entity %d\n", EntityCount++);
					fprintf(MAPFILE, "{\n\"classname\" \"light\"");
					fprintf(MAPFILE, "\n\"name\" \"light_1\"", EntityCount);
					switch (levelNo)
						{
							case 0:
								fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 3840.0, 250.0, 410.0); break;
								//case 1:
								//	fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 895.0, 4620.0, 370.0); break;
							case 2:
								fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 660.0, 255.0, 424.0); break;
						}

					fprintf(MAPFILE, "\n\"light_center\" \"0 0 0\"");
					fprintf(MAPFILE, "\n\"light_radius\" \"320 320 320\"");
					fprintf(MAPFILE, "\n\"_color\" \"0.45 0.45 0.45\"");
					fprintf(MAPFILE, "\n\"nodiffuse\" \"0\"");
					fprintf(MAPFILE, "\n\"noshadows\" \"0\"");
					fprintf(MAPFILE, "\n\"nospecular\" \"0\"");
					fprintf(MAPFILE, "\n\"parallel\" \"0\"");
					fprintf(MAPFILE, "\n}\n");
					}
				break;
			case UW2:
				if (levelNo <= 0)
					{//Temp code for providing a player start on level one as well as a "head" light.
					fprintf(MAPFILE, "// entity %d\n", EntityCount++);
					fprintf(MAPFILE, "{\n\"classname\" \"info_player_start\"");
					fprintf(MAPFILE, "\n\"name\" \"info_player_start\"");
					fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 2465.0, 5794.5, 360.5);
					fprintf(MAPFILE, "\n}\n");

					fprintf(MAPFILE, "// entity %d\n", EntityCount++);
					fprintf(MAPFILE, "{\n\"classname\" \"light\"");
					fprintf(MAPFILE, "\n\"name\" \"light_1\"", EntityCount);
					switch (levelNo)
						{
							case 0:
								fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 2465.0, 5794.5, 410.0); break;
								//case 1:
								//	fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 902, 4622, 432.0); break;
								//case 2:
								//	fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 660, 255, 432.0); break;
						}
					fprintf(MAPFILE, "\n\"light_center\" \"0 0 0\"");
					fprintf(MAPFILE, "\n\"light_radius\" \"320 320 320\"");
					fprintf(MAPFILE, "\n\"_color\" \"0.45 0.45 0.45\"");
					fprintf(MAPFILE, "\n\"nodiffuse\" \"0\"");
					fprintf(MAPFILE, "\n\"noshadows\" \"0\"");
					fprintf(MAPFILE, "\n\"nospecular\" \"0\"");
					fprintf(MAPFILE, "\n\"parallel\" \"0\"");
					fprintf(MAPFILE, "\n}\n");
					}
				break;
			case SHOCK:
				//Speaker for playing back logs
				fprintf(MAPFILE, "// entity %d\n", EntityCount++);
				fprintf(MAPFILE, "{\n\"classname\" \"atdm:voice\"");
				fprintf(MAPFILE, "\n\"name\" \"data_reader_voice\"");
				fprintf(MAPFILE, "\n\"origin\" \"%d %d 120\"", 32 * BrushSizeX, 32 * BrushSizeY);	//May cause leaks on small maps.
				fprintf(MAPFILE, "\n\"s_shader\" \"silence\"");
				fprintf(MAPFILE, "\n}\n");

				fprintf(MAPFILE, "// entity %d\n", EntityCount++);
				fprintf(MAPFILE, "{\n\"classname\" \"atdm:trigger_voice\"");
				fprintf(MAPFILE, "\n\"name\" \"data_reader_trigger\"");
				fprintf(MAPFILE, "\n\"origin\" \"%d %d 120\"", 32 * BrushSizeX, 32 * BrushSizeY);	//May cause leaks on small maps.
				fprintf(MAPFILE, "\n\"snd_say\" \"silence\"");
				fprintf(MAPFILE, "\n\"target0\" \"data_reader_voice\"");
				fprintf(MAPFILE, "\n\"as_player\" \"1\"");
				fprintf(MAPFILE, "\n}\n");
				if (levelNo == 1)
					{//startposition for player
					fprintf(MAPFILE, "// entity %d\n", EntityCount++);
					fprintf(MAPFILE, "{\n\"classname\" \"info_player_start\"");
					fprintf(MAPFILE, "\n\"name\" \"info_player_start\"");
					fprintf(MAPFILE, "\n\"origin\" \"%f %f %f\"", 3642.25, 2722.75, 175.75);
					fprintf(MAPFILE, "\n}\n");
					}
				break;
		}

	}


void getWallTextureName(tile t, int face, short waterWall)
	{
	//Spits out the wall texture code in the .map format
	//Water is a special case here and in SHOCK the texture needs to be offset vertically
	int wallTexture;
	int textureOffset = 1;
	int ceilOffset = 0;
	float scaleFactor = 1;
	float shiftFactorH = 0;
	wallTexture = t.wallTexture;
	if (iGame == SHOCK)
		{ //I need to calculate an offset for SHOCK.
		textureOffset = t.shockTextureOffset;
		ceilOffset = t.ceilingHeight;
		}
	if ((t.isWater != 1) || (waterWall == 0))
		{
		switch (face)
			{//Pick which texture is rendered for the selected face. Each tile has a north,south,east and west value for it's texture.
				case fNORTH:
					wallTexture = t.North;
					if ((iGame == SHOCK))
						{
						textureOffset = t.shockNorthOffset;
						ceilOffset = t.shockNorthCeilHeight;
						}
					break;
				case fSOUTH:
					wallTexture = t.South;
					if ((iGame == SHOCK))
						{
						textureOffset = t.shockSouthOffset;
						ceilOffset = t.shockSouthCeilHeight;
						}
					break;
				case fWEST:
					wallTexture = t.West;
					if ((iGame == SHOCK))
						{
						textureOffset = t.shockWestOffset;
						ceilOffset = t.shockWestCeilHeight;
						}
					break;
				case fEAST:
					wallTexture = t.East;
					if ((iGame == SHOCK))
						{
						textureOffset = t.shockEastOffset;
						ceilOffset = t.shockEastCeilHeight;
						}
					break;

			}
		if (wallTexture >512) { wallTexture = CAULK; }//Special case for a map bug?
		switch (wallTexture)
			{//Some special hard coded textures.
				case TRIGGER_MULTI:	//For trigger entities.
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/trigmulti\" 0 0 0\n"); break; }
				case NODRAW:	//nodraw
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/nodraw\" 0 0 0\n"); break; }
				case VISPORTAL:	//visportal
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/editor/visportal\" 0 0 0\n"); break; }
				case CAULK:
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/caulk\" 0 0 0\n"); break; }
				case COLLISION:
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/collision\" 0 0 0\n"); break; }
				default:
					{//The actual textures.
					if ((t.tileType >= TILE_DIAG_SE) && (t.tileType <= TILE_DIAG_NW) && (face == fSELF))
						{//The angled portion of a diagonal tile. Calculate how much the texture is stretched here
						scaleFactor = calcAlignmentFactor(BrushSizeX, BrushSizeY);
						switch (t.tileType)
							{
								case TILE_DIAG_SE:
									if ((t.tileX - t.tileY) % 2 == 0)
										{
										shiftFactorH = 0;
										}
									else
										{
										shiftFactorH = 0.5;
										}
									break;
								case TILE_DIAG_SW:
									if ((t.tileX - t.tileY) % 2 == 0)
										{
										shiftFactorH = 0.5;
										}
									else
										{
										shiftFactorH = 0;
										}
									break;
								case TILE_DIAG_NE:
									if ((t.tileX - t.tileY) % 2 == 0)
										{
										shiftFactorH = 0.5;
										}
									else
										{
										shiftFactorH = 0;
										}
									break;
								case TILE_DIAG_NW:
									if ((t.tileX - t.tileY) % 2 == 0)
										{
										shiftFactorH = 0;
										}
									else
										{
										shiftFactorH = 0.5;
										}
									break;
							}
						}
					if (iGame == SHOCK)
						{//The stepping of texture vertical positions can only be 8 different values. I shift the value down to get the remainder
						//Really should use a division here for this.
						float shock_ceil = SHOCK_CEILING_HEIGHT;
						float floorOffset = shock_ceil - ceilOffset - 8;	//The floor of the tile if it is 1 texture tall.
						while (floorOffset >= 8)	//Reduce the offset to 0 to 7 since textures go up in steps of 1/8ths
							{
							floorOffset -= 8;
							}
						float textureVertAlign = (floorOffset) / 8;
						//Write the texture to file
						fprintf(MAPFILE, "( ( %f %f %f ) ( %f %f %f ) ) \"",
							textureMasters[wallTexture].align1_1 / scaleFactor, textureMasters[wallTexture].align1_2, shiftFactorH,
							textureMasters[wallTexture].align2_1, textureMasters[wallTexture].align2_2, textureVertAlign);
						}
					else
						{//Texture aligned with the ceiling
						//Write the texture to file
						fprintf(MAPFILE, "( ( %f %f %f ) ( %f %f %f ) ) \"",
							textureMasters[wallTexture].align1_1 / scaleFactor, textureMasters[wallTexture].align1_2, shiftFactorH,
							textureMasters[wallTexture].align2_1, textureMasters[wallTexture].align2_2, textureMasters[wallTexture].align2_3);
						}
					fprintf(MAPFILE, "%s", textureMasters[wallTexture].path);//The texture path
					fprintf(MAPFILE, "\" 0 0 0\n");

					}
			}
		}
	else	//Water tile
		{
		switch (face)
			{
				case fNORTH:
					wallTexture = t.North; break;
				case fSOUTH:
					wallTexture = t.South; break;
				case fWEST:
					wallTexture = t.West; break;
				case fEAST:
					wallTexture = t.East; break;
			}
		if (wallTexture >512) { wallTexture = CAULK; }//Special case
		switch (wallTexture)
			{
				case TRIGGER_MULTI:
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/trigmulti\" 0 0 0\n"); break; }
				case NODRAW:	//nodraw
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/nodraw\" 0 0 0\n"); break; }
				case VISPORTAL:	//visportal
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/editor/visportal\" 0 0 0\n"); break; }
				case CAULK:
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/caulk\" 0 0 0\n"); break; }
				case COLLISION:
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/caulk\" 0 0 0\n"); break; }
				default:
					{//Actual textures.
					fprintf(MAPFILE, "( ( %f %f %f ) ( %f %f %f ) ) \"", textureMasters[wallTexture].align1_1, textureMasters[wallTexture].align1_2, textureMasters[wallTexture].align1_3, textureMasters[wallTexture].align2_1, textureMasters[wallTexture].align2_2, textureMasters[wallTexture].align2_3);
					fprintf(MAPFILE, "%s", textureMasters[wallTexture].path);
					fprintf(MAPFILE, "\" 0 0 0\n");
					}
			}
		}
	return;
	}

void getFloorTextureName(tile t, int face)
	{
	//Spits out the floor texture for a tile based on the face.

	int floorTexture;

	//My alignment params after recalculation based on the files in config.
	float floorAlign1;
	float floorAlign2;
	float floorAlign3;
	float floorAlign4;
	float floorAlign5;
	float floorAlign6;

	if (face == fCEIL)
		{
		floorTexture = t.shockCeilingTexture;
		}
	else
		{
		floorTexture = t.floorTexture;
		}



	if (floorTexture <0)
		{
		switch (floorTexture)
			{
				case TRIGGER_MULTI:
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/trigmulti\" 0 0 0\n"); break; }
				case NODRAW:	//nodraw
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/nodraw\" 0 0 0\n"); break; }
				case VISPORTAL:	//visportal
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/editor/visportal\" 0 0 0\n"); break; }
				case COLLISION:
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/collision\" 0 0 0\n"); break; }
				case CAULK:
				default:
					{fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/caulk\" 0 0 0\n"); break; }
			}
		}
	else
		{
		if (floorTexture >= 513)
			{
			fprintf(MAPFILE, "( ( 0 0.03125 0 ) ( -0.03125 0 0 ) ) \"textures/common/caulk\" 0 0 0\n");
			}
		else
			{
			//default values first.
			floorAlign1 = textureMasters[floorTexture].floor_align1_1;
			floorAlign2 = textureMasters[floorTexture].floor_align1_2;
			floorAlign3 = textureMasters[floorTexture].floor_align1_3;
			floorAlign4 = textureMasters[floorTexture].floor_align2_1;
			floorAlign5 = textureMasters[floorTexture].floor_align2_2;
			floorAlign6 = textureMasters[floorTexture].floor_align2_3;

			//Calc the values of the texture params for the slope
			CalcSlopedTextureAlignments(t, face, floorTexture, &floorAlign1, &floorAlign2, &floorAlign3, &floorAlign4, &floorAlign5, &floorAlign6);


			fprintf(MAPFILE, "( ( %f %f %f ) ( %f %f %f ) ) \"",
				floorAlign1, floorAlign2, floorAlign3,
				floorAlign4, floorAlign5, floorAlign6);

			fprintf(MAPFILE, "%s", textureMasters[floorTexture].path);
			fprintf(MAPFILE, "\" 0 0 0\n");
			}
		}

	return;
	}

void RenderFloorAndCeiling(int game, tile LevelInfo[64][64])
	{
	/*Renders the ceiling for UW and a floor underneath the level to stop water entities leaking*/
	switch (game)
		{
			case SHOCK:
				{//UW only
				return;
				break;
				}
			default:
				{
				//Create a temp tile that is the size of the level. This is used to render the foor at the bottom and the ceiling.
				tile tmp;
				tmp.tileType = 1;
				tmp.Render = 1;
				tmp.isWater = 0;
				tmp.tileX = 0;
				tmp.tileY = 0;
				tmp.DimX = 64;
				tmp.DimY = 64;
				tmp.ceilingHeight = 0;
				tmp.floorTexture = LevelInfo[0][0].shockCeilingTexture;
				tmp.shockCeilingTexture = LevelInfo[0][0].shockCeilingTexture;
				tmp.East = LevelInfo[0][0].shockCeilingTexture;//CAULK;
				tmp.West = LevelInfo[0][0].shockCeilingTexture;//CAULK;
				tmp.North = LevelInfo[0][0].shockCeilingTexture;//CAULK;
				tmp.South = LevelInfo[0][0].shockCeilingTexture;//CAULK;
				RenderDarkModTile(game, 0, 0, tmp, 0, 0, 1, 0);


				//and a floor 
				fprintf(MAPFILE, "// primitive %d\n", PrimitiveCount++);
				fprintf(MAPFILE, "{\nbrushDef3\n{\n");
				//001	east face -absdist
				fprintf(MAPFILE, "( 1 0 0 %d ) ( ( 0.03125 0 0 ) ( 0 0.03125 0 ) ) \"textures/common/caulk\" 0 0 0\n", -(64 * BrushSizeX));
				//010 north face -absdist
				fprintf(MAPFILE, "( 0 1 0 %d ) ( ( 0.03125 0 0 ) ( 0 0.03125 0 ) ) \"textures/common/caulk\" 0 0 0\n", -(64 * BrushSizeY));
				//100	top face -absdist
				//		fprintf (MAPFILE, "( 0 0 1 %d ) ( ( 0.03125 0 0 ) ( 0 0.03125 0 ) ) \"textures/darkmod/stone/natural/tiling_1d/gravel_red_01\" 0 0 0\n", +(2*BrushSizeZ) );	
				fprintf(MAPFILE, "( 0 0 1 %d )", +2 * BrushSizeZ);	//to go underneath
				getFloorTextureName(tmp, fCEIL);
				//00-1	west face +absdist
				fprintf(MAPFILE, "( -1 0 0 %d ) ( ( 0.03125 0 0 ) ( 0 0.03125 0 ) ) \"textures/common/caulk\" 0 0 0\n", +(0));
				//0-1 0	south face +absdist
				fprintf(MAPFILE, "( 0 -1 0 %d ) ( ( 0.03125 0 0 ) ( 0 0.03125 0 ) ) \"textures/common/caulk\" 0 0 0\n", +(0));
				//-100	bottom face +absdist
				fprintf(MAPFILE, "( 0 0 -1 %d ) ( ( 0.03125 0 0 ) ( 0 0.03125 0 ) ) \"textures/common/caulk\" 0 0 0\n", -(2 * BrushSizeZ) - 10);
				fprintf(MAPFILE, "}\n}\n");
				break;
				}
		}
	}



void RenderObjectList(int game, tile LevelInfo[64][64], ObjectItem objList[1600])
	{
	/*Parses the object list and sets up their x,y,z position.
	Object lists in-game are linked lists. The index into the list is stored on the tilemap*/
	int x; int y;
	float offX; float offY; float offZ;
	for (y = 0; y <= 63; y++)
		{
		for (x = 0; x <= 63; x++)
			{
			if ((LevelInfo[x][y].indexObjectList != 0))
				{
				long nextObj = LevelInfo[x][y].indexObjectList;
				while (nextObj != 0)
					{
					objList[nextObj].tileX = x;//Set the tile X and Y of the object. This is usefull to know.
					objList[nextObj].tileY = y;
					CalcObjectXYZ(game, &offX, &offY, &offZ, LevelInfo, objList, nextObj, x, y);//Figures out where the object should be.
					if (objList[nextObj].AlreadyRendered != 1)
						{
						RenderEntity(game, offX, offY, offZ, objList[nextObj], objList, LevelInfo);//Creates my object.
						}
					objList[nextObj].AlreadyRendered = 1;//Prevent possible duplication of objects due to system shock supporting objects that take occupy multiple tiles
					nextObj = objList[nextObj].next;//In linked list.
					}
				}
			}
		}
	}

void RenderLevelExits(int game, tile LevelInfo[64][64], ObjectItem objList[1600])
	{
	//A level exit in UW is a teleporter that refers to another level.
	int i;
	//go through the objects and find teleport traps with a zpos !=0
	for (i = 0; i<1600; i++)
		{
		if ((isTrigger(objList[i])) && (objectMasters[objList[objList[i].link].item_id].type == A_TELEPORT_TRAP))
		if ((objList[objList[i].link].zpos != 0))	//Trap goes to another level
			{
			//first a trigger to reference in scripting.
			fprintf(MAPFILE, "// entity %d\n", EntityCount++);
			fprintf(MAPFILE, "{\n\"classname\" \"trigger_relay\"\n");
			fprintf(MAPFILE, "\"name\" \"%s_%03d_%03d\"\n", "trigger_endlevel", objList[i].tileX, objList[i].tileY);
			fprintf(MAPFILE, "\n\"origin\" \"2500 2500 120\"\n");
			fprintf(MAPFILE, "\"target\" \"%s_%03d_%03d\"\n", "endlevel", objList[i].tileX, objList[i].tileY);
			fprintf(MAPFILE, "}\n");
			//And an endlevel to move to the next one.
			fprintf(MAPFILE, "// entity %d\n", EntityCount++);
			fprintf(MAPFILE, "{\n\"classname\" \"target_endlevel\"\n");
			fprintf(MAPFILE, "\"name\" \"%s_%03d_%03d\"\n", "endlevel", objList[i].tileX, objList[i].tileY);
			fprintf(MAPFILE, "\n\"origin\" \"2500 2500 120\"\n");
			switch (game)
				{
					case UWDEMO:
						fprintf(MAPFILE, "\n\"nextmap\" \"%s\"\n", "NO_SUCH_LEVEL"); break;
					case UW1:
						fprintf(MAPFILE, "\n\"nextmap\" \"%s_%d\"\n", "uw1", objList[objList[i].link].zpos - 1);	break;
					case UW2:
						fprintf(MAPFILE, "\n\"nextmap\" \"%s_%d\"\n", "uw2", objList[objList[i].link].zpos - 1);	break;
					case SHOCK:
						fprintf(MAPFILE, "\n\"nextmap\" \"%s_%d\"\n", "shock", objList[objList[i].link].zpos - 1);	break;
				}
			fprintf(MAPFILE, "}\n");
			}

		}

	//Now create matching entrance teleports. For this I'm using a file I generated from an ascii dump
	//TODO: This is a bit of a hack. I need to revisit.
	FILE *f = NULL;
	char line[255];
	const char *filePathE = ENTRANCES_CONFIG_FILE;
	f = fopen(filePathE, "r");
	if (f != NULL)
		{
		fgets(line, 254, f);//skip the first line
		while (fgets(line, 255, f))
			{
			int level = 0;
			int tileX = 0;
			int tileY = 0;

			sscanf(line, "%d %d %d", &level, &tileX, &tileY);
			if (level == levelNo)
				{	//on this level
				fprintf(MAPFILE, "// entity %d\n", EntityCount++);
				fprintf(MAPFILE, "{\n\"classname\" \"func_teleporter\"\n");
				fprintf(MAPFILE, "\"name\" \"%s_%03d_%03d\"\n", "entrance", tileX, tileY);
				fprintf(MAPFILE, "\n\"origin\" \"%d %d %d\"\n", tileX * BrushSizeX + (BrushSizeX / 2), tileY*BrushSizeY + (BrushSizeY / 2), (LevelInfo[tileX][tileY].floorHeight)* BrushSizeZ + 30);
				fprintf(MAPFILE, "}\n");
				}
			}
		fclose(f);
		}
	}

void RenderEntityElevator(int game, tile LevelInfo[64][64], ObjectItem &currobj)
	{
	//Creates an elevator. An elevator is just a tile rendered as a func static. The floor and ceiling can be moved independently in SHOCK. Floor only in UW.
	//	1 = floor only	(uw style)
	//	2 = ceiling only 
	//  3 = both	

	//floor
	fprintf(MAPFILE, "\n// entity %d\n{\n", EntityCount);
	fprintf(MAPFILE, "\"classname\" \"func_mover\"\n");
	fprintf(MAPFILE, "\"name\" \"floor_%03d_%03d\"\n", currobj.tileX, currobj.tileY);
	fprintf(MAPFILE, "\"model\" \"floor_%03d_%03d\"\n", currobj.tileX, currobj.tileY);
	fprintf(MAPFILE, "\"origin\" \"%d %d %d\"\n", currobj.tileX*BrushSizeX, currobj.tileY*BrushSizeY, 0);
	PrimitiveCount = 0;
	RenderDarkModTile(game, 0, 0, LevelInfo[currobj.tileX][currobj.tileY], 0, 0, 0, 1);
	fprintf(MAPFILE, "\n}");

	//ceiling
	EntityCount++;
	fprintf(MAPFILE, "\n// entity %d\n{\n", EntityCount);
	fprintf(MAPFILE, "\"classname\" \"func_mover\"\n");
	fprintf(MAPFILE, "\"name\" \"ceiling_%03d_%03d\"\n", currobj.tileX, currobj.tileY);
	fprintf(MAPFILE, "\"model\" \"ceiling_%03d_%03d\"\n", currobj.tileX, currobj.tileY);
	fprintf(MAPFILE, "\"origin\" \"%d %d %d\"\n", currobj.tileX*BrushSizeX, currobj.tileY*BrushSizeY, 0);
	PrimitiveCount = 0;
	RenderDarkModTile(game, 0, 0, LevelInfo[currobj.tileX][currobj.tileY], 0, 1, 1, 0);
	fprintf(MAPFILE, "\n}");
	EntityCount++;

	}

float calcAlignmentFactor(float adjacent, float opposite)
	{
	//Calculates the scale of the texture being rendered when sloped. What a misleading name!
	return  sqrt((adjacent*adjacent + opposite*opposite)) / adjacent;

	}

float getSteepOffset(int steepness)
	{
	//These are the multiplication factors for each level of steepness in aligning their textures on the slope.
	//Figures based on the 120 brush size. If changed these will need to be recalculated.
	switch (steepness)
		{
			case 1:return 1.969 / 128.0;
			case 2:return 3.764 / 128.0;
			case 3:return 5.26 / 128.0;
			case 4:return 6.4 / 128.0;
			case 5:return 7.191 / 128.0;
			case 6:return 7.68 / 128.0;
			case 7:return 7.929 / 128.0;
			case 8:return 8.0 / 128.0;
			case 9:return 7.944 / 128.0;
			case 10:return 7.804 / 128.0;
			case 12:return 7.384 / 128.0;
			case 11:return 7.6108 / 128.0;
			case 14:return 6.892 / 128.0;
			default:return 0;
		}
	}

void CalcSlopedTextureAlignments(tile t, int face, int floorTexture, float *floorAlign1, float *floorAlign2, float *floorAlign3, float *floorAlign4, float *floorAlign5, float *floorAlign6)
	{
	//Changes the the texture alignments of textures on slopes so they always point in the right direction.

	float BrushZ = (float)BrushSizeZ;
	float BrushX = (float)BrushSizeX;

	float scaleFactor = 1;	//For stretching floor textures.
	float shiftFactor = 1; //for aligning floor textures.

	scaleFactor = calcAlignmentFactor(BrushX, (float)t.shockSteep * BrushZ);
	switch (t.tileType)  //different tile orientations take different alignment values.
		{
			case TILE_OPEN:
				{
				//flip the fourth parameter since I'm an idiot who did'nt check his work
				*floorAlign4 = -*floorAlign4;
				break;
				}
			case TILE_SLOPE_N:
				{
				if (
					((face != fCEIL) && (t.shockSlopeFlag != SLOPE_CEILING_ONLY))
					)
					{
					*floorAlign1 = textureMasters[floorTexture].floor_align1_2;
					*floorAlign2 = textureMasters[floorTexture].floor_align1_3;
					*floorAlign3 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign4 = textureMasters[floorTexture].floor_align2_3;
					*floorAlign5 = -textureMasters[floorTexture].floor_align2_1 / scaleFactor;  //scale factor
					float shiftPoint = t.floorHeight - (t.tileY * t.shockSteep); //get a position where that slope intercects the axis
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign6 = 1 + shiftFactor;
					}
				else if ((face == fCEIL) && (t.shockSlopeFlag == SLOPE_BOTH_OPPOSITE))
					{//do t tile slope s alignment.
					*floorAlign1 = textureMasters[floorTexture].floor_align2_1;
					*floorAlign2 = textureMasters[floorTexture].floor_align2_2;
					*floorAlign3 = textureMasters[floorTexture].floor_align2_3;
					*floorAlign4 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign5 = +textureMasters[floorTexture].floor_align1_2 / scaleFactor;  //scale factor
					//fixme float shiftPoint = (t.tileY + 1) * t.shockSteep + (CEILING_HEIGHT - t.ceilingHeight - t.shockSteep);
					float shiftPoint;
					if (t.ActualType != t.tileType)
						{//Tile is a valley tile that has had it's type changed temporarily for rendering. It's ceiling is adjusted for differently!
						shiftPoint = (t.tileY + 1) * t.shockSteep + (CEILING_HEIGHT - t.ceilingHeight);
						}
					else
						{
						shiftPoint = (t.tileY + 1) * t.shockSteep + (CEILING_HEIGHT - t.ceilingHeight - t.shockSteep);
						}

					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign6 = +shiftFactor;
					}
				else if ((face == fCEIL) && ((t.shockSlopeFlag == SLOPE_BOTH_PARALLEL) || (t.shockSlopeFlag == SLOPE_CEILING_ONLY)))
					{//a north slope from the ceiling.
					*floorAlign1 = textureMasters[floorTexture].floor_align1_2;
					*floorAlign2 = textureMasters[floorTexture].floor_align1_3;
					*floorAlign3 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign4 = textureMasters[floorTexture].floor_align2_3;
					*floorAlign5 = -textureMasters[floorTexture].floor_align2_1 / scaleFactor;  //scale factor
					float shiftPoint = (CEILING_HEIGHT - t.ceilingHeight - t.shockSteep) - (t.tileY * t.shockSteep); //get a position where that slope intercects the axis
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign6 = 1 + shiftFactor;
					}

				break;
				}
			case TILE_SLOPE_S:
				{
				if (
					((face != fCEIL) && (t.shockSlopeFlag != SLOPE_CEILING_ONLY))
					)
					{
					*floorAlign1 = textureMasters[floorTexture].floor_align2_1;
					*floorAlign2 = textureMasters[floorTexture].floor_align2_2;
					*floorAlign3 = textureMasters[floorTexture].floor_align2_3;
					*floorAlign4 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign5 = +textureMasters[floorTexture].floor_align1_2 / scaleFactor;  //scale factor
					float shiftPoint = (t.tileY + 1) * t.shockSteep + t.floorHeight;
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign6 = +shiftFactor;
					}
				else if ((face == fCEIL) && (t.shockSlopeFlag == SLOPE_BOTH_OPPOSITE))
					{//do t tile slope n alignment.#
					*floorAlign1 = textureMasters[floorTexture].floor_align1_2;
					*floorAlign2 = textureMasters[floorTexture].floor_align1_3;
					*floorAlign3 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign4 = textureMasters[floorTexture].floor_align2_3;
					*floorAlign5 = +textureMasters[floorTexture].floor_align2_1 / scaleFactor;  //scale factor
					//float shiftPoint = (CEILING_HEIGHT - t.ceilingHeight - t.shockSteep) - (t.tileY * t.shockSteep); //get a position where that slope intercects the axis
					float shiftPoint;
					if (t.ActualType != t.tileType)
						{//Tile is a valley tile that has had it's type changed temporarily for rendering. It's ceiling is adjusted for differently!
						shiftPoint = (CEILING_HEIGHT - t.ceilingHeight) - (t.tileY * t.shockSteep); //get a position where that slope intercects the axis
						}
					else
						{
						shiftPoint = (CEILING_HEIGHT - t.ceilingHeight - t.shockSteep) - (t.tileY * t.shockSteep); //get a position where that slope intercects the axis
						}
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign6 = +shiftFactor;
					}
				else if ((face == fCEIL) && ((t.shockSlopeFlag == SLOPE_BOTH_PARALLEL) || (t.shockSlopeFlag == SLOPE_CEILING_ONLY)))
					{//The a south slope from the ceiling.
					*floorAlign1 = textureMasters[floorTexture].floor_align2_1;
					*floorAlign2 = textureMasters[floorTexture].floor_align2_2;
					*floorAlign3 = textureMasters[floorTexture].floor_align2_3;
					*floorAlign4 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign5 = +textureMasters[floorTexture].floor_align1_2 / scaleFactor;  //scale factor
					float shiftPoint = (t.tileY + 1) * t.shockSteep + (CEILING_HEIGHT - t.ceilingHeight - t.shockSteep);
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign6 = +shiftFactor;
					}
				break;
				}

			case TILE_SLOPE_E:
				{
				if (
					((face != fCEIL) && (t.shockSlopeFlag != SLOPE_CEILING_ONLY))
					)
					{
					*floorAlign1 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign2 = textureMasters[floorTexture].floor_align2_1 / scaleFactor;   //vert scale
					float shiftPoint = -t.floorHeight + (t.tileX * t.shockSteep); //get a position where that slope intercects the axis
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign3 = shiftFactor;// 	//horz Shift factor.textureMasters[floorTexture].floor_align1_3;
					*floorAlign4 = textureMasters[floorTexture].floor_align1_2;  //horz scale factor
					*floorAlign5 = textureMasters[floorTexture].floor_align2_2;
					*floorAlign6 = textureMasters[floorTexture].floor_align2_3;
					}
				else if ((face == fCEIL) && (t.shockSlopeFlag == SLOPE_BOTH_OPPOSITE))
					{//do t tile slope w alignment.#
					*floorAlign1 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign2 = textureMasters[floorTexture].floor_align1_2 / scaleFactor;  //scale factor
					//float shiftPoint = (t.tileX + 1) * t.shockSteep + (CEILING_HEIGHT - t.ceilingHeight - t.shockSteep);
					float shiftPoint;
					if (t.ActualType != t.tileType)
						{
						shiftPoint = (t.tileX + 1) * t.shockSteep + (CEILING_HEIGHT - t.ceilingHeight);
						}
					else
						{
						shiftPoint = (t.tileX + 1) * t.shockSteep + (CEILING_HEIGHT - t.ceilingHeight - t.shockSteep);
						}
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign3 = 1 + shiftFactor; // textureMasters[floorTexture].floor_align1_3; //Shift factor.
					*floorAlign4 = textureMasters[floorTexture].floor_align2_1;
					*floorAlign5 = textureMasters[floorTexture].floor_align2_2;
					*floorAlign6 = textureMasters[floorTexture].floor_align2_3;
					}
				else if ((face == fCEIL) && ((t.shockSlopeFlag == SLOPE_BOTH_PARALLEL) || (t.shockSlopeFlag == SLOPE_CEILING_ONLY)))
					{//The a east slope from the ceiling. 
					*floorAlign1 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign2 = textureMasters[floorTexture].floor_align2_1 / scaleFactor;   //vert scale
					float shiftPoint = -(CEILING_HEIGHT - t.ceilingHeight - t.shockSteep) + (t.tileX * t.shockSteep); //get a position where that slope intercects the axis
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign3 = shiftFactor;// 	//horz Shift factor.textureMasters[floorTexture].floor_align1_3;
					*floorAlign4 = textureMasters[floorTexture].floor_align1_2;  //horz scale factor
					*floorAlign5 = textureMasters[floorTexture].floor_align2_2;
					*floorAlign6 = textureMasters[floorTexture].floor_align2_3;
					}
				else
					{
					printf("x");
					}

				break;
				}

			case TILE_SLOPE_W:
				{
				if (
					((face != fCEIL) && (t.shockSlopeFlag != SLOPE_CEILING_ONLY))
					)
					{
					*floorAlign1 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign2 = textureMasters[floorTexture].floor_align1_2 / scaleFactor;  //scale factor
					float shiftPoint = (t.tileX + 1) * t.shockSteep + t.floorHeight;
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign3 = 1 + shiftFactor; // textureMasters[floorTexture].floor_align1_3; //Shift factor.
					*floorAlign4 = textureMasters[floorTexture].floor_align2_1;
					*floorAlign5 = textureMasters[floorTexture].floor_align2_2;
					*floorAlign6 = textureMasters[floorTexture].floor_align2_3;
					}
				else if ((face == fCEIL) && (t.shockSlopeFlag == SLOPE_BOTH_OPPOSITE))
					{//do t tile slope e alignment.
					*floorAlign1 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign2 = textureMasters[floorTexture].floor_align2_1 / scaleFactor;   //vert scale
					//float shiftPoint = -(CEILING_HEIGHT - t.ceilingHeight - t.shockSteep) + (t.tileX * t.shockSteep); //get a position where that slope intercects the axis
					float shiftPoint;
					if (t.ActualType != t.tileType)
						{
						shiftPoint = -(CEILING_HEIGHT - t.ceilingHeight) + (t.tileX * t.shockSteep); //get a position where that slope intercects the axis
						}
					else
						{
						shiftPoint = -(CEILING_HEIGHT - t.ceilingHeight - t.shockSteep) + (t.tileX * t.shockSteep); //get a position where that slope intercects the axis
						}
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign3 = shiftFactor;// 	//horz Shift factor.textureMasters[floorTexture].floor_align1_3;
					*floorAlign4 = textureMasters[floorTexture].floor_align1_2;  //horz scale factor
					*floorAlign5 = textureMasters[floorTexture].floor_align2_2;
					*floorAlign6 = textureMasters[floorTexture].floor_align2_3;
					}
				else if ((face == fCEIL) && ((t.shockSlopeFlag == SLOPE_BOTH_PARALLEL) || (t.shockSlopeFlag == SLOPE_CEILING_ONLY)))
					{// a west slope from the ceiling.
					*floorAlign1 = textureMasters[floorTexture].floor_align1_1;
					*floorAlign2 = textureMasters[floorTexture].floor_align1_2 / scaleFactor;  //scale factor
					float shiftPoint = (t.tileX + 1) * t.shockSteep + (CEILING_HEIGHT - t.ceilingHeight - t.shockSteep);
					shiftFactor = getSteepOffset(t.shockSteep) * (float)shiftPoint;
					*floorAlign3 = 1 + shiftFactor; // textureMasters[floorTexture].floor_align1_3; //Shift factor.
					*floorAlign4 = textureMasters[floorTexture].floor_align2_1;
					*floorAlign5 = textureMasters[floorTexture].floor_align2_2;
					*floorAlign6 = textureMasters[floorTexture].floor_align2_3;
					}
				else
					{
					printf("x");
					}

				break;
				}
			default:
				{
				*floorAlign1 = textureMasters[floorTexture].floor_align1_1;
				*floorAlign2 = textureMasters[floorTexture].floor_align1_2;
				*floorAlign3 = textureMasters[floorTexture].floor_align1_3;
				*floorAlign4 = textureMasters[floorTexture].floor_align2_1;
				*floorAlign5 = textureMasters[floorTexture].floor_align2_2;
				*floorAlign6 = textureMasters[floorTexture].floor_align2_3;
				break;
				}
		}


	}

void RenderPillars(int game, tile LevelInfo[64][64], ObjectItem objList[1600])
	{
	//Renders a pillar as a brush. I can't use an object here since it needs to extend to variable heights etc.
	int x; int y;
	float offX; float offY; float offZ;

	for (y = 0; y <= 63; y++)
		{
		for (x = 0; x <= 63; x++)
			{
			if ((LevelInfo[x][y].indexObjectList != 0))
				{
				long nextObj = LevelInfo[x][y].indexObjectList;
				while (nextObj != 0)
					{
					if (objectMasters[objList[nextObj].item_id].type == PILLAR)
						{
						int objType = objectMasters[objList[nextObj].item_id].type;
						CalcObjectXYZ(game, &offX, &offY, &offZ, LevelInfo, objList, nextObj, x, y);	//Gets its position.
						//Draw it around this point
						int textureIndex = objList[nextObj].flags & 0x3F;	//Bottom 2 bits give the index of the pillar texture
						fprintf(MAPFILE, "// primitive %d\n", PrimitiveCount++);
						fprintf(MAPFILE, "{\nbrushDef3\n{\n");
						//East face
						fprintf(MAPFILE, "( 1 0 0 %f )", -(offX + 5));
						getObjectTextureName(game, textureIndex, fEAST, objType);
						//North face
						fprintf(MAPFILE, "( 0 1 0 %f ) ", -(offY + 5));
						getObjectTextureName(game, textureIndex, fNORTH, objType);
						//Top face
						fprintf(MAPFILE, "( 0 0 1 %d )", -(UW_CEILING_HEIGHT - LevelInfo[x][y].ceilingHeight)* BrushSizeZ);
						getObjectTextureName(game, textureIndex, fTOP, objType);
						//West face
						fprintf(MAPFILE, "( -1 0 0 %f )", +(offX - 5));
						getObjectTextureName(game, textureIndex, fWEST, objType);
						//South face
						fprintf(MAPFILE, "( 0 -1 0 %f )", +(offY - 5));
						getObjectTextureName(game, textureIndex, fSOUTH, objType);
						//Bottom face 
						fprintf(MAPFILE, "( 0 0 -1 %d ) ", LevelInfo[x][y].floorHeight * BrushSizeZ);
						getObjectTextureName(game, textureIndex, fBOTTOM, objType);
						fprintf(MAPFILE, "}\n}\n");

						}
					nextObj = objList[nextObj].next;
					}
				}
			}
		}
	}

void getObjectTextureName(int game, int textureIndex, int face, int objType)
	{
	//Gets the special texture for specified object.

	switch (objType)
		case PILLAR:
		{//0.125 0 0 ) ( 0 0.03125 0 
		fprintf(MAPFILE, "( ( %f %f %f ) ( %f %f %f ) ) \"", 0.125, 0.0, 0.0, 0.0, 0.03125, 0.0);
		switch (game)
			{
				case UWDEMO:
				case UW1:
					{fprintf(MAPFILE, "textures/uw1/tmobj/tmobj_%02d", textureIndex); break; }
				case UW2:
					{fprintf(MAPFILE, "textures/uw2/tmobj/tmobj_%02d", textureIndex); break; }
			}
		fprintf(MAPFILE, "\" 0 0 0\n");
		break;
		}
	}