/*
 * This file is part of NumptyPhysics
 * Copyright (C) 2008 Tim Edmonds
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#include "Common.h"
#include "Array.h"
#include "Config.h"
#include "Game.h"
#include "Path.h"
#include "Canvas.h"
#include "Font.h"
#include "Levels.h"
#include "Http.h"
#include "Os.h"
#include "Scene.h"
#include "Script.h"
#include "Dialogs.h"
#include "Ui.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory.h>
#include <errno.h>
#include <sys/stat.h>

using namespace std;

unsigned char levelbuf[64*1024];


#define JOINT_IND_PATH "282,39 280,38 282,38 285,39 300,39 301,60 303,66 302,64 301,63 300,48 297,41 296,42 294,43 293,45 291,46 289,48 287,49 286,52 284,53 283,58 281,62 280,66 282,78 284,82 287,84 290,85 294,88 297,88 299,89 302,90 308,90 311,89 314,89 320,85 321,83 323,83 324,81 327,78 328,75 327,63 326,58 325,55 323,54 321,51 320,49 319,48 316,46 314,44 312,43 314,43"




class Game : public GameControl, public Container
{
  Scene   	    m_scene;
  Stroke  	   *m_createStroke;
  Stroke           *m_moveStroke;
  Widget           *m_pauseLabel;
  Widget           *m_editLabel;
  Widget           *m_completedDialog;
  Widget           *m_options;
  Os               *m_os;
  bool              m_isCompleted;
  Path              m_jointCandidates;
  Path              m_jointInd;
public:
  Game( Levels* levels, int width, int height ) 
  : m_createStroke(NULL),
    m_moveStroke(NULL),
    m_pauseLabel( NULL ),
    m_editLabel( NULL ),
    m_completedDialog( NULL ),
    m_isCompleted(false),
    m_options( NULL ),
    m_os( Os::get() ),
    m_jointInd(JOINT_IND_PATH)
  {
    setEventMap(Os::get()->getEventMap(GAME_MAP));
    sizeTo(Vec2(width,height));
    transparent(true); //don't clear
    m_greedyMouse = true; //get mouse clicks outside the window!

    m_jointInd.scale( 12.0f / (float32)m_jointInd.bbox().width() );
    //m_jointInd.simplify( 2.0f );
    m_jointInd.makeRelative();
    configureScreenTransform( width, height );
    m_levels = levels;
    gotoLevel(0);
    //add( new Button("O",Event::OPTION), Rect(800-32,0,32,32) );
  }


  const char* name() {return "Game";}

  void gotoLevel( int level, bool replay=false )
  {
    bool ok = false;
    m_replaying = replay;

    if ( replay ) {
      // reset scene, delete user strokes, but retain log
      m_scene.reset( NULL, true );
      m_scene.start( true );
      ok = true;
    } else if ( level >= 0 && level < m_levels->numLevels() ) {
      int size = m_levels->load( level, levelbuf, sizeof(levelbuf) );
      if ( size && m_scene.load( levelbuf, size ) ) {
	m_scene.start( m_scene.getLog()->size() > 0 );
	ok = true;
      }
    }

    if (ok) {
      //m_window.setSubName( file );
      if ( m_edit ) {
	m_scene.protect(0);
      } else {
      }
      m_refresh = true;
      m_level = level;
      if (!m_replaying) {
	m_stats.reset(SDL_GetTicks());
      }
    }
  }


  bool save( const char *file=NULL )
  {	  
    string p;
    if ( file ) {
      p = file;
    } else {
      p = Config::userDataDir() + Os::pathSep + "L99_saved.nph";
      file = "L99_saved.nph";
    }
    if ( m_scene.save( p ) ) {
      m_levels->addPath( p.c_str() );
      int l = m_levels->findLevel( p.c_str() );
      if ( l >= 0 ) {
	fprintf(stderr,"setting level to saved index to %d\n", l);
	m_level = l;
	//m_window.setSubName( p.c_str() );
      }
      showMessage(std::string("<P align=center>saved to<BR>")+file);
      return true;
    }
    return false;
  }


  bool send()
  {
    if ( save( SEND_TEMP_FILE ) ) {
      Http h;
      if ( h.post( (Config::planetRoot()+"/upload").c_str(),
		   "data", SEND_TEMP_FILE, "type=1" ) ) {
	std::string id = h.getHeader("NP-Upload-Id");
	if ( id.length() > 0 ) {
	  printf("uploaded as id %s\n",id.c_str());
	  if ( !m_os->openBrowser((Config::planetRoot()+"/editlevel?id="+id).c_str()) ) {
	    showMessage("Unable to launch browser");
	  }
	} else {
	  showMessage("UploadFailed: unknown error");
	}
      } else {
	showMessage(std::string("UploadFailed: ")+h.errorMessage());
      }
    }
    return false;
  }

  void saveDemo()
  {
    std::string path = m_levels->demoPath(m_level);
    OS->ensurePath(path);
    path = m_levels->demoName(m_level);
    fprintf(stderr,"saving demo of level %d to %s\n",
	    m_level, path.c_str());
    m_scene.save(path, true);
  }

  void clickMode(int cm)
  {
    if (cm != m_clickMode) {
      fprintf(stderr,"clickMode=%d!\n",cm);
      m_clickMode = cm;
      switch (cm) {
      case 1: setEventMap(Os::get()->getEventMap(GAME_MOVE_MAP)); break;
      case 2: setEventMap(Os::get()->getEventMap(GAME_ERASE_MAP)); break;
      default: setEventMap(Os::get()->getEventMap(GAME_MAP)); break;
      }
    }
  }

  void setTool( int t )
  {
    m_colour = t;
  }

  void editMode( bool set )
  {
    m_edit = set;
  }

  void showMessage( const std::string& msg )
  {
    printf("showMessage \"%s\"\n",msg.c_str());
    add( new MessageBox(msg) );
  }

  void togglePause()
  {
    if ( !m_paused ) {
      if ( !m_pauseLabel ) {
      	m_pauseLabel = new Button("PAUSED",Event::PAUSE);
	m_pauseLabel->setBg(0xff0000);
	m_pauseLabel->setFg(0x000000);
      }
      add( m_pauseLabel, Rect(0,0,128,32) );
      m_paused = true;
    } else {
      remove( m_pauseLabel );
      m_pauseLabel = NULL;
      m_paused = false;
    }
  }

  bool isPaused()
  {
    return m_paused;
  }

  void edit( bool doEdit )
  {
    if ( m_edit != doEdit ) {
      m_edit = doEdit;
      if ( m_edit ) {	
	if ( !m_editLabel ) {
 	  m_editLabel = new Button("EDIT",Event::DONE);
	  m_editLabel->setBg(0xff0000);
	  m_editLabel->setFg(0x000000);
 	}
	add(m_editLabel, Rect(SCREEN_WIDTH-128,0,SCREEN_WIDTH,32));
	m_scene.protect(0);
      } else {
	remove(m_editLabel);
	m_editLabel = NULL;
	m_strokeFixed = false;
	m_strokeSleep = false;
	m_strokeDecor = false;
	if ( m_colour < 2 ) m_colour = 2;
	m_scene.protect();
      }
    }
  }

  Vec2 mousePoint( Event& ev )
  {
    Vec2 pt( ev.x, ev.y );
    worldToScreen.inverseTransform( pt );
    return pt;
  }


  ////////////////////////////////////////////////////////////////
  // layer interface
  ////////////////////////////////////////////////////////////////

  virtual bool isDirty()
  {
    //TODO this can be a bit heavyweight
    return Container::isDirty() || !dirtyArea().isEmpty();
  }

  virtual Rect dirtyArea() 
  {
    //todo include dirt  for old joint candidates
    m_jointCandidates.empty();
    if ( m_refresh  ) {
      if ( m_createStroke ) {
	//this messes up dirty calc so do _after_ dirty area eval.
	m_scene.getJointCandidates( m_createStroke, m_jointCandidates );
      }
      return FULLSCREEN_RECT;
    } else {
      Rect r = m_scene.dirtyArea();
      if ( m_createStroke ) {
	//this messes up dirty calc so do _after_ dirty area eval.
	m_scene.getJointCandidates( m_createStroke, m_jointCandidates );
	if ( m_jointCandidates.size() ) {
	  Rect jr = m_jointCandidates.bbox();
	  jr.grow( 8 );
	  r.expand( jr );
	}
      }
      r.grow(8);
      r.expand(Container::dirtyArea());
      return r;
    }
  }

  void remove( Widget* w )
  {
    if (w==m_completedDialog) {
      m_completedDialog = NULL;
    }
    Container::remove(w);
  }

  virtual void onTick( int tick ) 
  {
    m_scene.step( isPaused() );

    if ( m_isCompleted && m_completedDialog && m_edit ) {
      remove( m_completedDialog );
      m_completedDialog = NULL;
      m_isCompleted = false;
    }
    if ( m_scene.isCompleted() != m_isCompleted && !m_edit ) {
      m_isCompleted = m_scene.isCompleted();
      if ( m_isCompleted ) {
	if (m_stats.endTime==0) {
	  //don't overwrite time after replay
	  m_stats.endTime = SDL_GetTicks();
	}
	fprintf(stderr,"STATS:\ttime=%dms\n\t"
		"strokes=%d (%d paused, %d undone)\n",
		m_stats.endTime-m_stats.startTime, m_stats.strokeCount,
		m_stats.pausedStrokes, m_stats.undoCount);
	m_completedDialog = createNextLevelDialog(this);
	add( m_completedDialog );
	saveDemo();
      } else if (m_completedDialog) {
	remove( m_completedDialog );
	m_completedDialog = NULL;
      }
    }

    if ( m_os ) {
      for ( char *f = m_os->getLaunchFile(); f; f=m_os->getLaunchFile() ) {
	if ( strstr(f,".npz") ) {
	  //m_levels->empty();
	}
	m_levels->addPath( f );
	int l = m_levels->findLevel( f );
	if ( l >= 0 ) {
	  gotoLevel( l );
	  //m_window.raise();
	}
      }    
    }  
    Container::onTick(tick);
  }

  virtual void draw( Canvas& screen, const Rect& area )
  {
    static int drawCount = 0 ;
    m_refresh = false;
    m_scene.draw( screen, area );
    if ( m_jointCandidates.size() ) {
      float32 rot = (float32)(drawCount&127) / 128.0f;
      for ( int i=0; i<m_jointCandidates.size(); i++ ) {
	Path joint = m_jointInd;
	joint.translate( -joint.bbox().centroid() );
	joint.rotate( b2Mat22(rot*2.0*3.141) );
	joint.translate( m_jointCandidates[i] + joint.bbox().centroid() );
	screen.drawPath( joint, 0x606060 );
      }
      drawCount++;
    }
    if ( m_fade ) {
      screen.fade( area );
    }
    Container::draw(screen,area);
  }

  virtual bool processEvent( SDL_Event& ev )
  {
    Event opt1Event(Event::OPTION,1);
    Event opt2Event(Event::OPTION,2);
    if (ev.type==SDL_MOUSEBUTTONDOWN) {
      if (ev.button.x < 10
	  && dispatchEvent(opt1Event)) {
	return true;
      } else if (ev.button.x > SCREEN_WIDTH-10
		 && dispatchEvent(opt2Event)) {
	return true;
      }
    }
    return Container::processEvent(ev);
  }

  virtual bool onEvent( Event& ev )
  {
    bool used = true;
    switch (ev.code) {
    case Event::MENU:
      remove( m_completedDialog );
      add( createMainMenu(this) );
      break;
    case Event::PAUSE:
      fprintf(stderr,"Game pause!\n");
      togglePause();
      break;
    case Event::UNDO:
      if ( !m_replaying ) {
	if ( m_createStroke ) {
	  m_scene.deleteStroke( m_createStroke );
	  m_createStroke = NULL;
	} else if ( m_scene.deleteStroke( m_scene.strokes().at(m_scene.strokes().size()-1) ) ) {
	  m_stats.undoCount++;
	}
      }
      m_refresh = true;
      break;
    case Event::SAVE:
      save();
      break;
    case Event::CANCEL:
      if ( m_edit ) {
	edit(false);
      }
      break;
    case Event::OPTION:
      remove( m_options );
      if (ev.x == 1) {
	//edit menu
	m_options = createEditOpts(this);
      } else if (ev.x == 2) {
	//play menu
	m_options = createPlayOpts(this);
      }
      if (m_options) {
	add( m_options );
      }
      break;
    case Event::SELECT:
      switch (ev.x) {
      case 1:
	switch (ev.y) {
	case -1:
	  add( createColourDialog(this, NUM_BRUSHES, brushColours) ); 
	  break;
	default:
	  fprintf(stderr,"SetTool %d\n",ev.y);
	  setTool(ev.y);
	  break;
	}
	break;
      case 2:
	switch (ev.y) {
	case -1:
	  add( createToolDialog(this) );
	  break;
	}	    
	break;
      }
      break;
    case Event::EDIT:
      edit( !m_edit );
      if (m_edit && !m_paused) {
	togglePause();
      }
      break;
    case Event::DONE:
      if (m_edit) {
	add( createEditDoneDialog(this) );
      }
      break;
    case Event::RESET:
      gotoLevel( m_level );
      break;
    case Event::NEXT:
      if (m_level==0 && m_isCompleted) {
	// from title try to find the first uncompleted level
	while (m_level < m_levels->numLevels()
	       && m_os->exists(m_levels->demoName(m_level))) {
	  m_level++;
	}
	gotoLevel( m_level );	
      } else {
	gotoLevel( m_level+1 );
      }
      break;
    case Event::PREVIOUS:
      gotoLevel( m_level-1 );
      break;
    case Event::REPLAY:
      gotoLevel( ev.x, true );
      break;
    case Event::PLAY:
      gotoLevel( ev.x );
      break;
    case Event::DRAWBEGIN:
      if ( !m_replaying && !m_createStroke ) {
	int attrib = 0;
	if ( m_strokeFixed ) attrib |= ATTRIB_GROUND;
	if ( m_strokeSleep ) attrib |= ATTRIB_SLEEPING;
	if ( m_strokeDecor ) attrib |= ATTRIB_DECOR;
	m_createStroke = m_scene.newStroke( Path()&mousePoint(ev), 
					    m_colour, attrib );
      }
      break;
    case Event::DRAWMORE:
      if ( m_createStroke ) {
	m_scene.extendStroke( m_createStroke, mousePoint(ev) );
      }
      break;
    case Event::DRAWEND:
      if ( m_createStroke ) {
	if ( m_scene.activateStroke( m_createStroke ) ) {
	  m_stats.strokeCount++;
	  if ( isPaused() ) {
	    m_stats.pausedStrokes++; 
	  }
	} else {
	  m_scene.deleteStroke( m_createStroke );
	}
	m_createStroke = NULL;
      }
      break;
    case Event::MOVEBEGIN:
      fprintf(stderr,"MOVING!\n");
      if ( !m_replaying && !m_moveStroke ) {
	m_moveStroke = m_scene.strokeAtPoint( mousePoint(ev),
					      SELECT_TOLERANCE );
      }
      break;
    case Event::MOVEMORE:
      if ( m_moveStroke ) {
	fprintf(stderr,"MOVING MORE!\n");
	m_scene.moveStroke( m_moveStroke, mousePoint(ev) );
      }
      break;
    case Event::MOVEEND:
      m_moveStroke = NULL;
      break;
    case Event::DELETE:
      fprintf(stderr,"DELETEING!\n");
      m_scene.deleteStroke( m_scene.strokeAtPoint( mousePoint(ev),
						   SELECT_TOLERANCE ) );
      m_refresh = true;
      break;
    default:
      used = Container::onEvent(ev);
    }
    return used;
  }

};



Widget* createGameLayer( Levels* levels, int width, int height )
{
  return new Game(levels,width,height);
}
