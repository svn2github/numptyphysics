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

#include "Dialogs.h"
#include "Ui.h"
#include "Canvas.h"
#include "Font.h"
#include "Config.h"
#include "Game.h"
#include "Scene.h"



const char * const HELP_TEXT = 
  "<H1>What To Do?</H1>"
  "Harness gravity with your crayon and set about creating blocks, ramps, levers, pulleys and whatever else you fancy to get the little red thing to the little yellow thing.<P>"

  "<H1>Drawing Stuff</H1>"
  "<H2>Strokes</H2>"
  "When you draw on the screen you create strokes. Each stroke is like a rigid piece of wire with a mass proportional to its length. A closed stroke is just a wire bent into a shape, it has no substance apart from its perimeter."
  "<H2>Joints</H2>"
  "The ends of a stroke can (and will) join onto other strokes when drawn near enough to another stroke. These joints are pivots so you can use this to build levers, pendulums, bicycles and other mechanical wonders. Jointed strokes can swing past each other without colliding. Join both ends to make a rigid structure."
  "<H2>Different Strokes</H2>"
  "The Tool Menu allows you to create some special types of strokes:<P>"
  "<LI>Ground -- a fixed stroke that does not move.<P>"
  "<LI>Sleepy -- a stroke that does not move until hit by another stroke.<P>"
  "<LI>Decor -- a decorative stroke that does not have a physical presence.<P>"
  "<H2>Cheating</H2>"
  "There's no such thing as cheating - it's all in your head.  Sometimes it's handy to pause the physics when making something complicated.  This is by no means necessary but try making a bicycle with live physics.  However it is commonly held that changing the pen colour and drawing a red thing right on top of the yellow thing is almost certainly a cheat's solution.<P>"
  "<H1>Controls</H1>"
  "Basic drawing is with finger or stylus.<P>"
  "Undo by tapping the proximity sensor or one of the undo Shortcut keys.<P>"
  "Swipe from the right for the Play Options.<P>"
  "Swipe from the left for the Game Options.<P>"
  "<H2>Shortcuts</H2>"
  "<LI>space or enter == Pause.<P>"
  "<LI>U, backspace, or down arrow -- Undo.<P>"
  "<LI>R or up arrow -- Reset.<P>"
  "<LI>N or right arrow -- Skip to next level.<P>"
  "<LI>P or left arrow -- Go back to previous level.<P>"
  "<H2>Play Options</H2>"
  "<LI>Pen -- change the colour of your pen.<P>"
  "<LI>Tool -- modify the how your strokes work.<P>"
  "<LI>Pause -- freeze time in the physics simulation.<P>"
  "<LI>Resume -- unfreeze time in the physics simulation.<P>"
  "<LI>Undo -- remove the last stroke.<P>"
  "<H2>Game Options</H2>"
  "<LI>Menu -- access the Main Menu.<P>"
  "<LI>Reset -- reset the level to its original state.<P>"
  "<LI>Skip -- skip to the next level.<P>"
  "<LI>Edit -- edit this level or create a new level.<P>"
  ;


////////////////////////////////////////////////////////////////


struct MenuPage : public Panel
{
  MenuPage(bool closeable=false)
  {
    alpha(100);
    Box *vbox = new VBox();
    m_content = new Panel();
    vbox->add( m_content, 100, 1 );
    add(vbox);
    fitToParent(true);
  }
  const char* name() {return "MenuPage";}
  Panel *m_content;
};

class LevelLauncher : public Dialog
{
public:
  LevelLauncher(int l, Canvas* c)
  {
    Box *vbox1 = new VBox();
    vbox1->add( new Spacer(),  100, 1 );
    Box *hbox = new HBox();
    hbox->add( new Spacer(),  10, 2 );
    IconButton *icon = new IconButton("level", "", Event::NOP);
    icon->canvas(c, false);
    hbox->add( icon, 300, 0 );
    hbox->add( new Spacer(),  10, 1 );
    Box *vbox = new VBox();
    vbox->add( new Spacer(),  10, 1 );
    vbox->add( new IconButton("Review","",
			      Event(Event::REPLAY,l)),
			      BUTTON_HEIGHT, 1 );
    vbox->add( new Spacer(),  10, 0 );
    vbox->add( new IconButton("Play","",
			      Event(Event::PLAY,l)),
			      BUTTON_HEIGHT, 1 );
    vbox->add( new Spacer(),  10, 1 );
    hbox->add( vbox, BUTTON_WIDTH, 0 );
    hbox->add( new Spacer(),  10, 2 );
    vbox1->add(hbox, 200, 0);
    vbox1->add( new Spacer(),  100, 1 );
    content()->add(vbox1);
    sizeTo(Vec2(SCREEN_WIDTH,SCREEN_HEIGHT));
    moveTo(Vec2(0,0));
    m_targetPos = Vec2( 0,0 );
  }
};

class LevelSelector : public MenuPage
{
  static const int THUMB_COUNT = 32;
  GameControl* m_game;
  Levels* m_levels;
  int m_collection;
  int m_dispbase;
  int m_dispcount;
  IconButton* m_thumbs[THUMB_COUNT];
  ScrollArea* m_scroll;
public:
  LevelSelector(GameControl* game, int initialLevel)
    : m_game(game),
      m_levels(game->m_levels),
      m_dispbase(0),
      m_dispcount(0),
      m_collection(0)
  {
    m_scroll = new ScrollArea();
    m_scroll->fitToParent(true);
    m_scroll->virtualSize(Vec2(SCREEN_WIDTH,SCREEN_HEIGHT));

    m_content->add(m_scroll,0,0);
    fitToParent(true);

    int levelInC;
    m_collection = m_levels->collectionFromLevel(initialLevel,&levelInC);
    setCollection(m_collection, levelInC);
  }
  void setCollection(int c, int levelInC)
  {
    if (c < 0 || c >=m_levels->numCollections()) {
      return;
    }    
    m_collection = c;
    m_dispbase = 0;
    m_dispcount = m_levels->collectionSize(c);
    m_scroll->virtualSize(Vec2(SCREEN_WIDTH,150+(SCREEN_HEIGHT/ICON_SCALE_FACTOR+40)*((m_dispcount+2)/3)));

    m_scroll->empty();
    Box *vbox = new VBox();
    vbox->add( new Spacer(),  10, 0 );
    Box *hbox = new HBox();
    Widget *w = new Button("<<",Event::PREVIOUS);
    w->border(false);
    hbox->add( w, BUTTON_WIDTH, 0 );
    hbox->add( new Spacer(), 10, 0 );
    Label *title = new Label(m_levels->collectionName(c));
    title->font(Font::headingFont());
    title->alpha(100);
    hbox->add( title, BUTTON_WIDTH, 4 );
    w= new Button(">>",Event::NEXT);
    w->border(false);    
    hbox->add( new Spacer(), 10, 0 );
    hbox->add( w, BUTTON_WIDTH, 0 );
    vbox->add( hbox, 64, 0 );
    vbox->add( new Spacer(),  10, 0 );

    hbox = new HBox();
    hbox->add( new Spacer(),  0, 1 );
    int accumw = 0;
    for (int i=0; i<m_dispcount; i++) {
      accumw += SCREEN_WIDTH / ICON_SCALE_FACTOR + 10;
      if (accumw >= SCREEN_WIDTH) {
	vbox->add(hbox, SCREEN_HEIGHT/ICON_SCALE_FACTOR+30, 4);
	vbox->add( new Spacer(),  10, 0 );
	hbox = new HBox();
	hbox->add( new Spacer(),  0, 1 );
	accumw = SCREEN_WIDTH / ICON_SCALE_FACTOR;
      }
      m_thumbs[i] = new IconButton("--","",Event(Event::PLAY, //SELECT,
						 m_levels->collectionLevel(c,i)));
      m_thumbs[i]->font(Font::blurbFont());
      m_thumbs[i]->setBg(SELECTED_BG);
      m_thumbs[i]->border(false);
      hbox->add( m_thumbs[i],  SCREEN_WIDTH / ICON_SCALE_FACTOR, 0 );
      hbox->add( new Spacer(), 0, 1 );
    }
    vbox->add(hbox, SCREEN_HEIGHT/ICON_SCALE_FACTOR+30, 4);
    vbox->add( new Spacer(), 110, 10 );
    m_scroll->add(vbox,0,0);

    for (int i=0; i<THUMB_COUNT && i+m_dispbase<m_dispcount; i++) {
      fprintf(stderr,"creating thumb\n");
      Canvas temp( SCREEN_WIDTH, SCREEN_HEIGHT );
      Scene scene( true );
      unsigned char buf[64*1024];
      int level = m_levels->collectionLevel(c,i);
      int size = m_levels->load( level, buf, sizeof(buf) );
      if ( size && scene.load( buf, size ) ) {
	scene.draw( temp, FULLSCREEN_RECT );
	m_thumbs[i]->text( m_levels->levelName(level) );
	m_thumbs[i]->canvas( temp.scale( ICON_SCALE_FACTOR ) );
	m_thumbs[i]->transparent(m_dispbase+i!=levelInC);
      }
    }
  }
  bool onEvent(Event& ev)
  {
    switch (ev.code) {
    case Event::PREVIOUS:
      setCollection(m_collection-1,0);
      return true;
    case Event::NEXT:
      setCollection(m_collection+1,0);
      return true;
//     case Event::SELECT:
//       for (int i=0; i<THUMB_COUNT && i+m_dispbase<m_dispcount; i++) {
// 	m_thumbs[i]->transparent(true);
//       }
//       m_thumbs[m_dispbase+ev.x]->transparent(false);
//       m_game->gotoLevel(m_levels->collectionLevel(m_collection,m_dispbase+ev.x));
//       add( new LevelLauncher(m_levels->collectionLevel(m_collection,m_dispbase+ev.x), m_thumbs[m_dispbase+ev.x]->canvas()) );
//       //Event closeEvent(Event::CLOSE);
//       //m_parent->dispatchEvent(closeEvent);
//       return true;
    }
    return MenuPage::onEvent(ev);
  }
};

class HelpPage : public MenuPage
{
public:
  HelpPage()
  {
    Box *vbox = new VBox();
    ScrollArea* scroll = new ScrollArea();
    scroll->fitToParent(true);
    RichText *text = new RichText(HELP_TEXT);
    scroll->virtualSize(Vec2(SCREEN_WIDTH,text->layout(SCREEN_WIDTH)));
    text->fitToParent(true);
    text->alpha(100);
    scroll->add(text,0,0);
    vbox->add( scroll, 0, 1 );
    vbox->add( new Button("http://numptyphysics.garage.maemo.org",Event::SELECT), 36, 0 );
    m_content->add(vbox,0,0);
  }
  bool onEvent(Event& ev)
  {
    if (ev.code == Event::SELECT) {
      OS->openBrowser("http://numptyphysics.garage.maemo.org");
      return true;
    }
    return Panel::onEvent(ev);
  }
};



struct FrontPage : public MenuPage
{
  FrontPage() : MenuPage(true)
  {
    m_content->add( new IconButton("CHOOSE","choose.png",
				   Event(Event::MENU,1)),
		    Rect(125,100,275,300) );
    m_content->add( new IconButton("PLAY","play.png",
				   Event(Event::MENU,2)),
		    Rect(325,100,475,300) );
    m_content->add( new IconButton("HELP","help.png",
				   Event(Event::MENU,3)),
		    Rect(525,100,675,300) );
    fitToParent(true);
  }
};


class MainMenu : public Dialog
{
  GameControl* m_game;
  int          m_chosenLevel;
public:
  MainMenu(GameControl* game)
    : Dialog("NUMPTY PHYSICS",Event::UNDO,Event::QUIT),
      m_game(game),
      m_chosenLevel(game->m_level)
  {
    content()->add(new FrontPage());
    m_targetPos = Vec2( 0, 0 );
    sizeTo(Vec2(SCREEN_WIDTH,SCREEN_HEIGHT));
  }
  bool onEvent( Event& ev )
  {
    switch (ev.code) {
    case Event::MENU:
      switch(ev.x) {
      case 1:
	content()->empty();	
	content()->add(new LevelSelector(m_game, m_chosenLevel));
	rightControl()->text("<--");
	rightControl()->event(Event::CANCEL);
	break;
      case 2:
	close();
	break;
      case 3: 
	content()->empty();
	content()->add(new HelpPage());
	rightControl()->text("<--");
	rightControl()->event(Event::CANCEL);
	break;
      }
      return true;
    case Event::SELECT:
      fprintf(stderr,"select level %d\n",ev.x);
      m_chosenLevel = ev.x;
      content()->empty();
      content()->add(new LevelLauncher(m_chosenLevel,NULL));
      rightControl()->text("<--");
      rightControl()->event(Event(Event::MENU,1));
      return true;
    case Event::CANCEL:
      content()->empty();
      content()->add(new FrontPage());
      rightControl()->text("X");
      rightControl()->event(Event::QUIT);
      return true;
    case Event::PLAY:
    case Event::REPLAY:
      close();
      break;
    default:
      break;
    }
    return Dialog::onEvent(ev);
  }
};


Widget* createMainMenu(GameControl* game)
{
  return new MainMenu(game);
}


////////////////////////////////////////////////////////////////


static const MenuItem playNormalOpts[] = {
  MenuItem("pen:pen.png",Event(Event::SELECT,1,-1)),
  MenuItem("tools:choose.png",Event(Event::SELECT,2,-1)),
  MenuItem("pause:pause.png",Event::PAUSE),
  MenuItem("undo:undo.png",Event::UNDO),
  MenuItem("",Event::NOP)
};

static const MenuItem playPausedOpts[] = {
  MenuItem("pen:pen.png",Event(Event::SELECT,1,-1)),
  MenuItem("tools:choose.png",Event(Event::SELECT,2,-1)),
  MenuItem("resume:play.png",Event::PAUSE),
  MenuItem("undo:undo.png",Event::UNDO),
  MenuItem("",Event::NOP)
};


class OptsPopup : public MenuDialog
{
public:
  OptsPopup() : MenuDialog(this, "", NULL) 
  {
    m_buttonDim = Vec2(90,90);
  }

  virtual Widget* makeButton( MenuItem* item, const Event& ev )
  {
    std::string file = item->text.substr(item->text.find(':')+1); 
    std::string label = item->text.substr(0,item->text.find(':')); 
    return new IconButton(label,file,ev);
  }
};


class PlayOpts : public OptsPopup
{
public:
  PlayOpts(GameControl* game )
  {
    addItems(game->m_paused ? playPausedOpts : playNormalOpts);
    sizeTo(Vec2(140,480));
    moveTo(Vec2(SCREEN_WIDTH,0));
    m_targetPos = Vec2(SCREEN_WIDTH-140,0);
  }
};

Widget* createPlayOpts(GameControl* game )
{
  return new PlayOpts(game);
}


////////////////////////////////////////////////////////////////


static const MenuItem editNormalOpts[] = {
  MenuItem("menu:close.png",Event::MENU),
  MenuItem("reset:reset.png",Event::RESET),
  MenuItem("skip:forward.png",Event::NEXT),
  MenuItem("edit:share.png",Event::EDIT),
  MenuItem("",Event::NOP)
};

static const MenuItem editDoneOpts[] = {
  MenuItem("menu:close.png",Event::MENU),
  MenuItem("reset:reset.png",Event::RESET),
  MenuItem("done:share.png",Event::DONE),
  MenuItem("",Event::NOP)
};


class EditOpts : public OptsPopup
{
public:
  EditOpts(GameControl* game )
  {
    addItems(game->m_edit ? editDoneOpts : editNormalOpts);
    sizeTo(Vec2(140,480));
    moveTo(Vec2(-140,0));
    m_targetPos = Vec2(0,0);
  }
};

Widget* createEditOpts(GameControl* game )
{
  return new EditOpts(game);
}


////////////////////////////////////////////////////////////////


class ColourButton : public Button
{
public:
  ColourButton(const std::string& s, int c, const Event& ev)
    : Button(s,ev)
  {
    m_bg = c;
    transparent(false);
  }
};

class ColourDialog : public MenuDialog
{
public:
  ColourDialog( int num, const int* cols ) 
    : MenuDialog(this,"pen"),
      m_colours(cols)					     
  {
    m_columns = 4;
    m_buttonDim = Vec2(BUTTON_HEIGHT, BUTTON_HEIGHT);
    for (int i=0; i<num; i++) {
      switch(i) {
      case 0: addItem( MenuItem("O",Event(Event::SELECT,1,i)) ); break;
      case 1: addItem( MenuItem("X",Event(Event::SELECT,1,i)) ); break;
      default: addItem( MenuItem("/",Event(Event::SELECT,1,i)) ); break;
      }
    }
    Vec2 size = m_buttonDim*5;
    sizeTo(size);
    m_targetPos = Vec2(SCREEN_WIDTH - m_pos.width(),0);
  }
  Widget* makeButton( MenuItem* item, const Event& ev )
  {
    Button *w = new ColourButton(item->text,m_colours[item->event.y],ev);
    w->font(Font::titleFont());
    return w;
  }
  const int* m_colours;
};

Widget* createColourDialog(GameControl* game, int n, const int* cols)
{
  return new ColourDialog(n,cols);
}


////////////////////////////////////////////////////////////////

static const MenuItem toolOpts[] = {
  MenuItem("ground:tick.png",Event(Event::SELECT,0)),
  MenuItem("sleepy:tick.png",Event(Event::SELECT,1)),
  MenuItem("decor:tick.png",Event(Event::SELECT,2)),
  MenuItem("move:tick.png",Event(Event::SELECT,3)),
  MenuItem("erase:tick.png",Event(Event::SELECT,4)),
  MenuItem("",Event::NOP)
};


class ToolDialog : public MenuDialog
{
public:
  ToolDialog(GameControl* game) : MenuDialog(this, "tools",NULL),
				  m_game(game)
  {
    addItems( toolOpts );
    updateTicks();
  }
  Widget* makeButton( MenuItem* item, const Event& ev )
  {
    std::string file = item->text.substr(item->text.find(':')+1); 
    std::string label = item->text.substr(0,item->text.find(':')); 
    IconButton *w = new IconButton(label,file,ev);
    w->align(1);
    m_opts.append(w);
    return w;
  }
  void empty()
  {
    m_opts.empty();
  }
  void remove( Widget* w )
  {
    if (w && m_opts.indexOf((IconButton*)w) >= 0) {
      m_opts.erase(m_opts.indexOf((IconButton*)w));
    }
  }
  void updateTicks()
  {
    for (int i=0; i<m_opts.size(); i++) {
      bool tick = false;
      if (m_opts[i]->text() == "ground") {
	tick = m_game->m_strokeFixed;
      } else if (m_opts[i]->text() == "sleepy") { 
	tick = m_game->m_strokeSleep;
      } else if (m_opts[i]->text() == "decor") { 
	tick = m_game->m_strokeDecor;
      } else if (m_opts[i]->text() == "move") { 
	tick = m_game->m_clickMode==1;
      } else if (m_opts[i]->text() == "erase") { 
	tick = m_game->m_clickMode==2;
      }
      m_opts[i]->icon(tick ? "tick.png" : "blank.png");
    }
  }
  bool onEvent( Event& ev )
  {
    switch (ev.code) {
    case Event::SELECT:
      switch(ev.x) {
      case 0:
	m_game->m_strokeFixed = !m_game->m_strokeFixed;
	m_game->m_strokeSleep = false;
	m_game->m_strokeDecor = false;
	break;
      case 1:
	m_game->m_strokeFixed = false;
	m_game->m_strokeSleep = !m_game->m_strokeSleep;
	m_game->m_strokeDecor = false;
	break;
      case 2:
	m_game->m_strokeFixed = false;
	m_game->m_strokeSleep = false;
	m_game->m_strokeDecor = !m_game->m_strokeDecor;
	break;
      case 3:
	m_game->clickMode((m_game->m_clickMode==1)?0:1);
	break;
      case 4:
	m_game->clickMode((m_game->m_clickMode==2)?0:2);
	break;
      default: return MenuDialog::onEvent(ev);
      }
      updateTicks();
      return true;
    }
    return MenuDialog::onEvent(ev);
  }
private:
  GameControl *m_game;
  Array<IconButton*> m_opts;
};



Widget* createToolDialog(GameControl* game)
{
  return new ToolDialog(game);
}


////////////////////////////////////////////////////////////////



class IconDialog : public MenuDialog
{
public:
  IconDialog( const std::string &title, const MenuItem* items=NULL ) 
    : MenuDialog(this,title)
  {
    // add items here to use correct buttons
    addItems(items);
  }
  Widget* makeButton( MenuItem* item, const Event& ev )
  {
    fprintf(stderr,"IconDialog::makeButton %s\n",item->text.c_str());
    try {
      return new IconButton(item->text,item->text+".png",ev);
    } catch (...) {
      return new Button(item->text,ev);
    }
  }
};

Widget *createIconDialog( const std::string &title, const MenuItem* items )
{
  return new IconDialog( title, items );
}


////////////////////////////////////////////////////////////////



class NextLevelDialog : public Dialog
{
  GameControl* m_game;
public:
  NextLevelDialog(GameControl* game)
    : Dialog("BRAVO!!!",Event::NOP,Event::MENU),
      m_game(game)
  {
    rightControl()->text("<--");
    char buf[32];
    const GameStats& stats = m_game->stats();
    int time = (stats.endTime - stats.startTime)/1000;
    int h = time/60/60;
    int m = time/60 - h*60;
    int s = time - m*60;

    Box *vbox = new VBox();
    vbox->add(new Spacer(),10,1);
    if (h > 0) {
      sprintf(buf,"time: %dh %dm %ds",m,h,s);
    } else if (m > 0) {
      int m = time/60/1000;
      sprintf(buf,"time: %dm %ds",m,s);
    } else {
      sprintf(buf,"time: %ds",s);
    }
    vbox->add(new Label(buf),20,0);
    sprintf(buf,"%d stroke%s",stats.strokeCount,stats.strokeCount==1?"":"s");
    vbox->add(new Label(buf),20,0);
    if (stats.pausedStrokes) {
      sprintf(buf,"     (%d while paused)",stats.pausedStrokes);
      vbox->add(new Label(buf),20,0);
    }
    sprintf(buf,"%d undo%s",stats.undoCount,stats.undoCount==1?"":"s");
    vbox->add(new Label(buf),20,0);
    vbox->add(new Spacer(),10,1);
 
    Box *hbox2 = new HBox();
    hbox2->add(new Spacer(),20,0);
    hbox2->add(new Button("review",Event(Event::REPLAY,game->m_level)),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),1,1);
    hbox2->add(new Button("again",Event::RESET),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),1,1);
    hbox2->add(new Button("next",Event::NEXT),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),20,0);
    vbox->add(hbox2,BUTTON_HEIGHT,0);

    vbox->add(new Spacer(),10,0);
    content()->add(vbox,0,0);
    m_targetPos = Vec2( 150, 70);
    sizeTo(Vec2(500,240));
  }
};


Widget *createNextLevelDialog( GameControl* game )
{
  return new NextLevelDialog(game);
}


////////////////////////////////////////////////////////////////

class EditDoneDialog : public Dialog
{
  GameControl* m_game;
public:
  EditDoneDialog(GameControl* game)
    : Dialog("Exit Editor",Event::NOP,Event::CLOSE),
      m_game(game)
  {
    Box *vbox = new VBox();
    vbox->add(new Spacer(),10,1);
    vbox->add(new Label("Save level?"),20,0);
    vbox->add(new Spacer(),10,1);
 
    Box *hbox2 = new HBox();
    hbox2->add(new Spacer(),20,0);
    hbox2->add(new Button("cancel",Event::CLOSE),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),1,1);
    hbox2->add(new Button("exit",Event::EDIT),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),1,1);
    hbox2->add(new Button("save",Event::SAVE),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),20,0);
    vbox->add(hbox2,BUTTON_HEIGHT,0);

    vbox->add(new Spacer(),10,0);
    content()->add(vbox,0,0);
    m_targetPos = Vec2( 150, 70);
    sizeTo(Vec2(500,240));
  }
  bool onEvent( Event& ev )
  {
    close();
    return false;
  }
};


Widget *createEditDoneDialog( GameControl* game )
{
  return new EditDoneDialog(game);
}

