//  $Id: start_tuxkart.cxx,v 1.53 2004/08/10 15:35:54 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <plib/pw.h>
#include <vector>
#include <string>

#include "tuxkart.h"

#include "TrackManager.h"
#include "StringUtils.h"
#include "WidgetSet.h"
#include "sound.h"
#include "Loader.h"

static ssgSimpleState *introMaterial ;

Characters characters;

void switchToGame ()
{
  delete introMaterial ;
  
  guiStack.clear();

  tuxkartMain () ;
}

/***********************************\
*                                   *
* This function redisplays the PUI, *
* and flips the double buffer.      *
*                                   *
\***********************************/

static void splashMainLoop (void)
{
  while ( 1 )
  {
    //Setup for boring 2D rendering
    glMatrixMode   ( GL_PROJECTION ) ;
    glLoadIdentity () ;
    glMatrixMode   ( GL_MODELVIEW ) ;
    glLoadIdentity () ;
    glDisable      ( GL_DEPTH_TEST ) ;
    glDisable      ( GL_LIGHTING   ) ;
    glDisable      ( GL_FOG        ) ;
    glDisable      ( GL_CULL_FACE  ) ;
    glDisable      ( GL_ALPHA_TEST ) ;
    //glOrtho        ( 0, 640, 0, 480, 0, 100 ) ;

    //Draw the splash screen
    introMaterial -> force () ;

    glBegin ( GL_QUADS ) ;
    glColor3f    ( 1, 1, 1 ) ;
    glTexCoord2f ( 0, 0 ) ; glVertex2i (   -1,   -1 ) ;
    glTexCoord2f ( 1, 0 ) ; glVertex2i (   1,   -1 ) ;
    glTexCoord2f ( 1, 1 ) ; glVertex2i (   1,   1 ) ;
    glTexCoord2f ( 0, 1 ) ; glVertex2i (   -1,   1 ) ;
    glEnd () ;

    glFlush();
	
    // Swapbuffers - and off we go again...
    pollEvents() ;
    updateGUI();
    swapBuffers();
  }
}


static void installMaterial ()
{
  /* Make a simplestate for the title screen texture */

  introMaterial = new ssgSimpleState ;
  introMaterial -> setTexture( 
          loader->createTexture("title_screen.png", true, true, false));
  introMaterial -> enable      ( GL_TEXTURE_2D ) ;
  introMaterial -> disable     ( GL_LIGHTING  ) ;
  introMaterial -> disable     ( GL_CULL_FACE ) ;
  introMaterial -> setOpaque   () ;
  introMaterial -> disable     ( GL_BLEND ) ;
  introMaterial -> setShadeModel ( GL_SMOOTH ) ;
  introMaterial -> disable     ( GL_COLOR_MATERIAL ) ;
  introMaterial -> enable      ( GL_CULL_FACE      ) ;
  introMaterial -> setMaterial ( GL_EMISSION, 0, 0, 0, 1 ) ;
  introMaterial -> setMaterial ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  introMaterial -> setMaterial ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
  introMaterial -> setMaterial ( GL_AMBIENT, 0, 0, 0, 1 ) ;
  introMaterial -> setShininess ( 0 ) ;
}

static void loadCharacters()
{
  std::set<std::string> result;
  loader->listFiles(result, "data/");

  // Findout which characters are available and load them
  for(std::set<std::string>::iterator i = result.begin(); i != result.end(); ++i)
    {
      if (StringUtils::has_suffix(*i, ".tkkf"))
        {
          characters.push_back(KartProperties("data/" + *i));
        }
    }
}

// Initialize the datadir
static void loadDataDir()
{
  loader = new Loader();
  loader->setModelDir("models");
  loader->setTextureDir("images");
    
  /* initialize search path of the loader */
  if ( getenv ( "TUXKART_DATADIR" ) != NULL )
    loader->addSearchPath(getenv ( "TUXKART_DATADIR" )) ;
  if (getenv ("HOME") != NULL) {
    std::string homepath = getenv("HOME");
    homepath += "/.tuxkart";
    loader->addSearchPath(homepath);
  }
  loader->addSearchPath(".");
  loader->addSearchPath("..");
  loader->addSearchPath(TUXKART_DATADIR);
}

// Load the datadir, tracklist and plib stuff
static void initTuxKart (int width, int height, int videoFlags)
{
  loadDataDir ();
  track_manager.loadTrackList () ;
  loadCharacters();

  initVideo ( width, height, videoFlags );
  
  // Initialise a bunch of PLIB library stuff
  ssgInit () ;
  ssgSetCurrentOptions(loader);
  registerImageLoaders();
  
  sound     = new SoundSystem ;
  widgetSet = new WidgetSet ;
}


/* Draw the startScreen */
void startScreen ( )
{
  guiStack.push_back(GUIS_MAINMENU);

  installMaterial () ;
  splashMainLoop  () ;
}


void cmdLineHelp (char* invocation)
{
  fprintf ( stdout, 
	    "Usage: %s [OPTIONS]\n\n"

	    "Run TuxKart, a racing game with go-kart that features"
	    " the well-known linux\nmascott Tux. The game is heavily"
	    " inspired by Super-Mario-Kart and Wacky Wheels.\n\n"

	    "Options:\n"
	    "  -N,  --no-start-screen  Quick race\n"
	    "  -t,  --track n          Start at track number n (see --list-tracks)\n"
            "  -k,  --numkarts NUM     Number of karts on the racetrack\n"
            "  --kart FILE             Use the kart defined in FILE (.tkkf)\n"
	    "  -l,  --list-tracks      Show available tracks.\n"
	    "  --laps n                Define number of laps to n\n"
	    "  --players n             Define number of players to either 1, 2 or 4.\n"
	    "  --reverse               Enable reverse mode\n"
	    "  --mirror                Enable mirror mode (when supported)\n"
	    "  -f,  --fullscreen       Fullscreen display.\n"
	    "  -s,  --screensize WIDTHxHEIGHT\n"
            "                          Set the screen size (e.g. 320x200)\n"
	    "  -v,  --version          Show version.\n"
	    "\n"
	    "You can visit TuxKart's homepage at "
	    "http://tuxkart.sourceforge.net\n\n", invocation
	    );
}


int main ( int argc, char *argv[] )
{
  /* Default values */
  int  width  = 800;
  int  height = 600;
  bool fullscreen   = false;
  bool noStartScreen = false;
  
  /* Testing if we've given arguments */
  if ( argc > 1) 
    {
      for(int i = 1; i < argc; ++i)
	{
	  if ( argv[i][0] != '-') continue;

	  if ( !strcmp(argv[i], "--help") or
	       !strcmp(argv[i], "-help") or
	       !strcmp(argv[i], "-h") )
	    {
	      cmdLineHelp(argv[0]);
	      return 0;
	    }

	  else if( (!strcmp(argv[i], "--kart") and argc > 2 ))
            {
              kart_props = KartProperties(argv[i+1]);
            }

	  else if( (!strcmp(argv[i], "--track") or !strcmp(argv[i], "-t")) and argc > 2 )
	    {
	      raceSetup.track = atoi(argv[i+1]);

	      if ( raceSetup.track < 0 )
		{
		  fprintf ( stderr,
			    "You choose an invalid track number: %d.\n", raceSetup.track );
		  cmdLineHelp(argv[0]);
		  return 0;
		}

	      fprintf ( stdout, "You choose to start in track: %s.\n", argv[i+1] ) ;
	    }

	  else if( (!strcmp(argv[i], "--numkarts") or !strcmp(argv[i], "-k")) && argc > 2)
	  {
	      raceSetup.numKarts = atoi(argv[i+1]);

	      fprintf ( stdout, "You choose to have %s karts.\n", argv[i+1] ) ;
	    }

	  else if( !strcmp(argv[i], "--list-tracks") or !strcmp(argv[i], "-l") )
	    {
	      loadDataDir ();
	      track_manager.loadTrackList () ;

	      fprintf ( stdout, "  Available tracks:\n" );
	      for (uint i = 0; i != track_manager.trackNames.size(); i++)
		       fprintf ( stdout, "\t%d: %s\n", i, track_manager.trackNames[i].c_str() );
	      
		fprintf ( stdout, "\n" );

	      return 0;
	    }

	  else if ( !strcmp(argv[i], "--no-start-screen") or !strcmp(argv[i], "-N") )
	    {
	      noStartScreen = true;
	    }

	  else if ( !strcmp(argv[i], "--reverse") )
	    {
	      fprintf ( stdout, "Enabling reverse mode.\n" ) ;
	      raceSetup.reverse = 1;
	    }

	  else if ( !strcmp(argv[i], "--mirror") )
	    {
#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
	      fprintf ( stdout, "Enabling mirror mode.\n" ) ;
	      raceSetup.mirror = 1;
#else
	      raceSetup.mirror = 0 ;
#endif
	    }

	  else if ( !strcmp(argv[i], "--laps") and argc > 2 )
	    {
	      fprintf ( stdout, "You choose to have %d laps.\n", atoi(argv[i+1]) ) ;
	      raceSetup.numLaps = atoi(argv[i+1]);
	    }

	  else if ( !strcmp(argv[i], "--players") and argc > 2 )
	    {
	      raceSetup.numPlayers = atoi(argv[i+1]);

	      if ( raceSetup.numPlayers < 0 or raceSetup.numPlayers > 4)
		{
		  fprintf ( stderr,
			    "You choose an invalid number of players: %d.\n",
			    raceSetup.numPlayers );
		  cmdLineHelp(argv[0]);
		  return 0;
		}

	      fprintf ( stdout, "You choose to have %d players.\n", atoi(argv[i+1]) ) ;
	    }

	  else if ( !strcmp(argv[i], "--fullscreen") or !strcmp(argv[i], "-f"))
	    {
              fullscreen = true;
	    }

	  else if ( !strcmp(argv[i], "--screensize") or !strcmp(argv[i], "-s") )
	    {
	      if (sscanf(argv[i+1], "%dx%d", &width, &height) == 2)
                fprintf ( stdout, "You choose to be in %dx%d.\n", width, height );
              else
                {
                  fprintf ( stderr, "Error: --screensize argument must be given as WIDTHxHEIGHT\n");
                  exit(EXIT_FAILURE);
                }
	    }
	  #ifdef VERSION
	  else if( !strcmp(argv[i], "--version") or  !strcmp(argv[i], "-v") )
	    {
	      fprintf ( stdout, "Tuxkart %s\n", VERSION ) ;
	      return 0;
	    }
	  #endif

	  else
	    {
	      fprintf ( stderr, "Invalid parameter: %s.\n\n", argv[i] );
	      cmdLineHelp(argv[0]);
	      return 0;
	    }
	}
    }

  initTuxKart ( width,  height, fullscreen );

  if ( noStartScreen )
      tuxkartMain ( ) ;
  else
    startScreen ( );

  return 0 ;
}
