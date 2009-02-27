#ifndef __CHARACK_MAP_GENERATOR_H_
#define __CHARACK_MAP_GENERATOR_H_

#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "CharackCoastGenerator.h"
#include "CharackLineSegment.h"
#include "vector3.h"

#define BLACK 0
#define BACK 1
#define GRID 2
#define WHITE 3
#define BLUE0 4

#define MAXCOL	10
typedef int CTable[MAXCOL][3];
    
#ifndef PI
	#define PI 3.14159265358979
#endif

#ifndef DEG2RAD
	#define DEG2RAD 0.0174532918661 /* pi/180 */
#endif

/**
 * Generates the map that all Charack classes will use to create the virtual world. The generated map
 * is a small representation of all elements in the world, such as continents and oceans.
 *
 * This class was created because its very hard to define land/water portions in the virtual world
 * using only math functions. Using the small representation generated by CharackMapGenerator, the
 * other classes can "guess" what to create in a specific position in the world (at least they will
 * know if the specific position is a piece of land or water).
 *
 * This class uses the continent generation algorithm by Torben AE. Mogensen <torbenm@diku.dk>. The original code
 * was downloaded from http://www.diku.dk/hjemmesider/ansatte/torbenm/, I just removed the code Charack will not
 * need and encapsulated the rest all togheter inside a class.
 */
class CharackMapGenerator {
	private:
		CharackCoastGenerator mCoastGen;
		int mCoastMap[CK_VIEW_FRUSTUM][CK_VIEW_FRUSTUM];

		int altColors;
		int BLUE1, LAND0, LAND1, LAND2, LAND4;
		int GREEN1, BROWN0, GREY0;
		int back;
		int nocols;
		int rtable[256], gtable[256], btable[256];
		int lighter; /* specifies lighter colours */
		double M;   /* initial altitude (slightly below sea level) */
		double dd1;   /* weight for altitude difference */
		double dd2;  /* weight for distance */
		double POW;  /* power for distance function */

		int Depth; /* depth of subdivisions */
		double r1,r2,r3,r4; /* seeds */
		double longi,lat,scale;
		double vgrid, hgrid;

		int latic; /* flag for latitude based colour */

		int Width;
		int Height;

		unsigned char **col;
		int **heights;
		int cl0[60][30];

		int do_outline;
		int do_bw;
		int *outx, *outy;

		double cla, sla, clo, slo;

		double rseed;
		double increment;

		int best;
		int weight[30];

		int min_dov(int x, int y);
		int max_dov(int x, int y);
		double fmin_dov(double x, double y);
		double fmax_dov(double x, double y);
		void setcolours();
		void makeoutline(int do_bw);
		void mercator();
		int planet0(double x, double y, double z);
		double planet(double a,double b,double c,double d, double as, double bs, double cs, double ds, double ax, double ay, double az, double bx, double by, double bz, double cx, double cy, double cz, double dx, double dy, double dz, double x, double y, double z, int level);
		double planet1(double x, double y, double z);
		double rand2(double p, double q);
		void printbmp(FILE *outfile);
		void printbmpBW(FILE *outfile);
		double log_2(double x);

		// Globaly check if a specific position is land or water. The test is made against the macro world information,
		// which means coast lines and similar things are not analyzed. The method isLand() will use the information
		// of globalIsLand() as a "clue" to identify the coast lines, so it can generate a highly detailed map
		// (with noised coast lines) that can be used to check if a specific position is land or water.
		int globalIsLand(float theX, float theZ);

		// Clean up all the information in the coast map. After this method invocation, every call
		// to isLand() will return false until applyCoast() is called (which will regenerate the land/water info).
		void clearCoastMap();

		// Find all coast lines visible on the screen.
		std::list<CharackLineSegment> findCoastLines(int theMapX, int theMapZ, int theViewFrustum, int theSample);

		// Apply all the cost point to the coast map, creating the lines among the points.
		void updateCoastMap(std::list<Vector3> theCoastPoints);

		// Starting in thePoint, this method will "walk" over the line thePoint belongs to, adding all found corners
		// to theCorners. To avoid walk over the lines already visited, the third parameter "theDirection" will guide
		// the movement direction. e.g: if the method is walkin in a vertical line, when a corner is found, the method
		// will switch the moviment direction to a horizontal one. When another corner is found on the horizontal line,
		// the moviment direction will be switched to vertical again, and so on.
		void findCorners(std::list<Vector3> theCorners, Vector3 thePoint, int theDirection, int theListInsertOrder);
		
		// Checks if theElement is in theList.
		int inList(std::list<Vector3> theList, Vector3 theElement);

		// Find a point that belongs to a coast line. The method will return the firt cost point found.
		Vector3 findCoast(int theMapX, int theMapZ, int theViewFrustum, int theSample);

		// Checks if a point is a corner or not.
		int isCorner(Vector3 thePoint);

		// Starting in theStart, the method will walk over the line thePoint belongs to until it finds the
		// corner at the end of the line. The param theGuideAxis tells which axis the method must use
		// to walk over. theDirection tells the direction in the axis, e.g: theGuieAxis=X, theDirection=RIGHT means
		// "walk over z axis of theStart, walking to the right, until you find a corver".
		//
		// theStart must belongs to a coast line.
		Vector3 findLineCorner(Vector3 theStart, int theDirection, int theGuideAxis);

		// thePoint must be a corner; the method will check if, starting with thePoint, is there a possibility
		// to "walk" down in a line. e.g:
		// 
		//   A --------
		//   |  land  |
		//   |        | 
		//   B --------
		//      water
		//
		// If thePoint is B, canGoDown() will return false. If thePoint is A, canGoDown() will return true.
		int canGoDown(Vector3 thePoint);

		// The same as canGoDown(), but checking left/right instead of up/down.
		int canGoLeft(Vector3 thePoint);

		// Checks if thePoint is inside the user view frustum.
		int isInsideViewFrustum(Vector3 thePoint);

		CharackCoastGenerator getCoastGenerator(void);

	public:
		static enum CLASS_DEFS {
			MOVE_DOWN		= -1,
			MOVE_UP			= 1,
			
			MOVE_LEFT		= -1,
			MOVE_RIGHT		= 1,

			AXIS_X			= 2,
			AXIS_Z			= 3,
			VERTICAL		= 4,
			HORIZONTAL		= 5,
			INSERT_BEGIN	= 6,
			INSERT_END		= 7
		};

		CharackMapGenerator();
		~CharackMapGenerator();

		// Generate the world (land and water)
		void generate();
		
		// Check if a specific position is land or water. 
		int isLand(float theX, float theZ);		

		// This method will find all coast lines (which are straight lines before the method call) and, for each one,
		// generate a much more real coast line, adding some noise to the lines.
		void applyCoast(int theMapX, int theMapZ, int theViewFrustum, int theSample);
};

#endif