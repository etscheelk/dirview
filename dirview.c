#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <string.h>
#include <strings.h>


#include <tickit.h>
#include <math.h>

#define streq(a,b) (!strcmp(a,b))

TickitPenRGB8 blue = {.r = 10, .g = 100, .b = 200};

/**
 * If I want to do fuzzy searching on local files, CLI tool fzf
 * 
 */

/**
 * I've never seen a struct created like this before.
 * 
 * I know if there is no name for the struct, it is named
 * for the name following its close brace and then a semicolon.
 * 
 * Based on `examples/demo-pen.c`, this creates an array called in this case
 * penColors with the helpful implicit struct creation {...}, wherein 
 * you can access the TickitPen* by name?? 
 * 
 * This seems to have proper properties of a tickit pen's attributes,
 * as can also be created by `TickitPen *tickit_pen_new_attrs(TickitPenAttr attr, ...)`.
 * 
 * I don't know, I'll have to test. 
 */
struct
{
	char *name;
	int val;

	TickitPen *pen_fg, *pen_fg_hi, *pen_bg, *pen_bg_hi;
} penColors[] = 
{
	{"blue", 1},
	{"red", 1}
};


TickitRect termRect;

TickitWindow *root;
TickitWindow *typer;

// Buffer for the text we store in the typer. Size determined by 2*cols.
// Declared first in typerOnExpose.
// free in main.
// Size is 2*cols for flexibility on window resize (not implemented currently)
char *typerBuffer; 
unsigned short numTyped = 0;

TickitKeyEventInfo lastKey;

// Function Declarations
// Events
static int rootOnExpose	(TickitWindow *win, TickitEventFlags flags, void *_info, void *data);
static int rootOnKey	(TickitWindow *win, TickitEventFlags flags, void *_info, void *data);
static int typerOnExpose(TickitWindow *win, TickitEventFlags flags, void *_info, void *data);


// TODO: Typer window at bottom, for user to type text in
// TODO: Find a way to make the program structure not fucked 
//			(headers? .o's?) (before it gets too late) (make a graph?)
// TODO: 

// Low priority or unimportant right now (focus on something else)
// TODO: Implement window resize considerations (low priority)


/**
 * @brief Expose event function called for root on spawn and any subsequent times expose is called. 
 * 
 * @param win 
 * @param flags 
 * @param _info 
 * @param data 
 * @return int 
 */
static int rootOnExpose (TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
	TickitExposeEventInfo *info = _info;
	TickitRenderBuffer *rb = info->rb;
	TickitRect rect = info->rect;

	int cols  = rect.cols,
		lines = rect.lines;

	int midx = cols / 2, 
		midy = lines / 2;

	


	// I don't know what this does
	tickit_renderbuffer_eraserect(rb, &info->rect);

	tickit_renderbuffer_goto(rb, 0, 0);
	

	tickit_renderbuffer_savepen(rb);


	if(!penColors[0].pen_fg)
      penColors[0].pen_fg = tickit_pen_new_attrs(TICKIT_PEN_FG, penColors[0].val, 0);
	
	
	// tickit_pen_set_colour_attr_rgb8(penColors[0].pen_fg, TICKIT_PEN_FG, (TickitPenRGB8){.r=10,.g=50,.b=200});


	TickitPen *circlePen = tickit_pen_new();
	
	bool a1 = tickit_pen_set_colour_attr_desc(circlePen, TICKIT_PEN_FG, "white");
	bool a2 = tickit_pen_set_colour_attr_desc(circlePen, TICKIT_PEN_BG, "hi-cyan");

	TickitPen *cursorPen = tickit_pen_new();
	tickit_pen_set_colour_attr_desc(cursorPen, TICKIT_PEN_FG, "hi-white");
	tickit_pen_set_colour_attr_desc(cursorPen, TICKIT_PEN_BG, "grey");
	tickit_pen_set_bool_attr(cursorPen, TICKIT_PEN_BLINK, true);
	
	tickit_pen_set_bool_attr(circlePen, (TickitPenAttr) TICKIT_PEN_STRIKE, false);
	tickit_renderbuffer_setpen(rb, circlePen);

	// tickit_renderbuffer_textf_at(rb, 2, 2, "NumCols: %3d\tNumLines: %3d", cols, lines);
	tickit_renderbuffer_textf(rb, "NumCols: %d NumLines: %d", cols, lines);
	// tickit_renderbuffer_text(rb, "hello world!");
	tickit_renderbuffer_textf_at(rb, 5, 5, "Hello there :)%s", " meow");

	tickit_renderbuffer_text_at(rb, 6, 5, "Escape\tcharacter?");

	tickit_renderbuffer_save(rb);
	for (int y = 0; y < lines; ++y)
	for (int x = 0; x < cols; ++x)
	{
		
		double dist = sqrt(pow((midx - x), 2) + pow((midy - y), 2));

		if (dist < 20.0)
		{
			tickit_renderbuffer_text_at(rb, y, x, "OO");
		}
	}
	tickit_renderbuffer_restore(rb);

	tickit_pen_set_colour_attr_desc(penColors[0].pen_fg, TICKIT_PEN_FG, "hi-cyan");
	tickit_renderbuffer_setpen(rb, penColors[0].pen_fg);
	// Command to render text to the screen. 
	// TODO: This doesn't print text yet and I don't know why
	//		--> I had this function as on_focus instead of on_expose
	tickit_renderbuffer_goto(rb, 30, 0);
	tickit_renderbuffer_textf(rb, "blue foreground? %s", penColors[0].name);

	for (int i = 0; i < 200; ++i)
	{
		tickit_renderbuffer_text(rb, ".");
	}

	// fancy line :)
	tickit_renderbuffer_hline_at(rb, 31, 0, 30, TICKIT_LINE_DOUBLE, (TickitLineCaps) 0);

	tickit_renderbuffer_restore(rb);

	tickit_renderbuffer_text(rb, "     ");


	tickit_renderbuffer_setpen(rb, cursorPen);
	tickit_renderbuffer_text_at(rb, lines-1, 0, "> ");

	return 1;
}

/**
 * @brief Event called when the root window experiences a key.
 * Modifies `lastKey` global variable.
 * Check if CTRL+Z was pressed, if so pauses the terminal. Can be replaced with `fg`. 
 * 
 * @param win root
 * @param flags 
 * @param _info TickitKeyEventInfo
 * @param data 
 * @return int 
 */
static int rootOnKey(TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
	TickitKeyEventInfo *info = _info;

	if(lastKey.str) free((void *)lastKey.str);

	lastKey = *info;
	lastKey.str = strdup(info->str);

	if (info->mod == TICKIT_MOD_CTRL && streq(info->str, "C-z")) 
	{
		TickitTerm *term = tickit_window_get_term(win);
		tickit_term_pause(term);
		raise(SIGSTOP);
		tickit_term_resume(term);
		tickit_window_expose(win, NULL);
		// return 0;
	}

	tickit_window_expose(typer, (TickitRect*) NULL);

	return 1;
}

static int typerOnExpose(TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
	TickitExposeEventInfo *info = _info;

	TickitRenderBuffer *rb = info->rb;
	TickitRect rect = info->rect;
	
	if (!typerBuffer)
	{
		typerBuffer = malloc(rect.cols * sizeof(char));
	}

	if (lastKey.str == NULL) return 0;

	

	// printf("typed exposed\n");

	tickit_renderbuffer_goto(rb, 0, 0);
	tickit_renderbuffer_clear(rb);

	TickitPen *p = tickit_pen_new();
	tickit_pen_set_colour_attr_desc(p, TICKIT_PEN_FG, "white");
	tickit_renderbuffer_setpen(rb, p);

	tickit_renderbuffer_text(rb, "> ");

	tickit_renderbuffer_text(rb, lastKey.str);
	// tickit_renderbuffer_textf(rb, "%d", tickit_renderbuffer_get_span(rb, 0, 0, NULL, NULL, 20));

	return 1;
}

int main(int argc, char *argv[]) 
{
	tickit_debug_enabled = true;
    printf("Welcome to dirview. It's probably not complete yet.\n");

    Tickit *t = tickit_new_stdtty();
    if(!t) 
	{
		fprintf(stderr, "Cannot create Tickit - %s\n", strerror(errno));
		return 1;
    }

	root = tickit_get_rootwin(t);
	if(!root) 
	{
		fprintf(stderr, "Cannot create root window - %s\n", strerror(errno));
		return 1;
  	}
	termRect = tickit_window_get_geometry(root);
	tickit_window_bind_event(root, TICKIT_WINDOW_ON_EXPOSE, (TickitBindFlags) 0, &rootOnExpose, NULL);
	// tickit_window_bind_event(root, TICKIT_WINDOW_ON_KEY, (TickitBindFlags) 0, &checksuspend, NULL);
	

	typer = tickit_window_new(root, (TickitRect){.top = termRect.lines-1, .left = 0, .lines = 2, .cols = termRect.cols}, (TickitWindowFlags) 0);
	if (!typer) 
	{
		fprintf(stderr, "Cannot create typer window - %s\n", strerror(errno));
		return 1;
	}
	tickit_window_bind_event(root, (TickitWindowEvent) TICKIT_WINDOW_ON_KEY, (TickitBindFlags) 0, &rootOnKey, NULL);
	tickit_window_bind_event(typer, TICKIT_WINDOW_ON_EXPOSE, (TickitBindFlags) 0, &typerOnExpose, NULL);
	
	tickit_run(t);

	tickit_window_close(root);

	tickit_unref(t);

	free(typerBuffer);

    return EXIT_SUCCESS;
}
