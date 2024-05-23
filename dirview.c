#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <string.h>
#include <strings.h>
#include <memory.h> // Microsoft safe memory movement

#include <tickit.h>
#include <math.h>

// Display names of all files in current directory
// https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
#include <dirent.h>

#define streq(a,b) (!strcmp(a,b))



/**
 * @brief Everything in bundle is the stuff that should be freed or unref'd
 * at end of program. Dereference or close in reverse order.
 */
static struct bundle
{
	//
	Tickit *t;

	//
	TickitWindow *root;

	//
	TickitWindow *typer;

	//
	TickitWindow *dirDisplay;

	// Buffer for the text we store in the typer. Size determined by 2*cols.
	// Declared first in typerOnExpose.
	// Size is cols for flexibility on window resize (not implemented currently)
	char *typerBuffer;
} bundle = {.t = NULL, .root = NULL, .typer = NULL, .dirDisplay = NULL, .typerBuffer = NULL};


static enum extraKey
{
	Normal,
	Backspace,
	Tab,
	PageUp,
	PageDown,
	Up,
	Down,
	Left,
	Right,
	Home,
	End,
	Insert,
	Delete,
	Function
};

static struct extraKeyInfo
{
	enum extraKey ex;
	
	// Optional for function key number
	uint8_t num;
} extraKeyInfo = {.ex = Normal};


static TickitPenRGB8 blue = {.r = 10, .g = 100, .b = 200};

static TickitRect termRect;

static uint16_t numTyped = 0;
static uint16_t typerWidth = 0;

static TickitKeyEventInfo lastKey;

// Function Declarations

static int rootOnExpose	(TickitWindow  *root, TickitEventFlags flags, void *_info, void *data);
static int rootOnKey	(TickitWindow  *root, TickitEventFlags flags, void *_info, void *data);
static int typerOnExpose(TickitWindow *typer, TickitEventFlags flags, void *_info, void *data);

static void empty_bundle(void);

/**
 * If I want to do fuzzy searching on local files, CLI tool fzf
 * 
 */






// TODO: Typer window at bottom, for user to type text in
// TODO: Find a way to make the program structure not fucked 
//			(headers? .o's?) (before it gets too late) (make a graph?)
// TODO: 

// Low priority or unimportant right now (focus on something else)
// TODO: Implement window resize considerations (low priority)


/**
 * @brief Expose event function called for root on spawn and any subsequent times expose is called. 
 * 
 * @param root 
 * @param flags 
 * @param _info 
 * @param data 
 * @return int 
 */
static int rootOnExpose (TickitWindow *root, TickitEventFlags flags, void *_info, void *data)
{
	TickitExposeEventInfo *info = _info;
	TickitRenderBuffer *rb = info->rb;
	TickitRect rect = info->rect;

	

	int cols  = rect.cols,
		lines = rect.lines;

	int midx = cols / 2, 
		midy = lines / 2;
	

	return 1;
}

/**
 * @brief Event called when the root window experiences a key.
 * Modifies `lastKey` global variable.
 * 
 * @details
 * Bindings:
 * 	CTRL + Z: pause, restore with fg.
 * 	ALT + Q : call exit function, quit. Safer than CTRL + C. 
 * 		aka Meta + Q if that's your poison
 * 
 * 
 * @param root root
 * @param flags 
 * @param _info TickitKeyEventInfo
 * @param data custom data
 * @return int 
 */
static int rootOnKey(TickitWindow *root, TickitEventFlags flags, void *_info, void *data)
{
	TickitKeyEventInfo *info = _info;

	

	if(lastKey.str) free((void *)lastKey.str);

	lastKey = *info;
	lastKey.str = strdup(info->str);

	if (info->mod == TICKIT_MOD_CTRL && streq(info->str, "C-z")) 
	{
		TickitTerm *term = tickit_window_get_term(root);
		tickit_term_pause(term);
		raise(SIGSTOP);
		tickit_term_resume(term);
		tickit_window_expose(root, NULL);
		return 1;
	}

	// if user types Meta-q or Meta-Q, quit the prorgram
	if (info->mod == TICKIT_MOD_ALT && (streq(info->str, "M-q") || streq(info->str, "M-Q")))
	{
		empty_bundle();
	}
	
	// If key isn't in combo with ALT or CTRL, send it to the typer
	if (!(info->mod & TICKIT_MOD_ALT) && !(info->mod & TICKIT_MOD_CTRL)) 
	{
		// Backspace
		if (streq(lastKey.str, "Backspace")) extraKeyInfo.ex = Backspace;

		// Enter
		// Tab
		// PageUp
		// PageDown
		// Delete
		// Home
		// Insert
		// Up
		// Down
		// Left
		// Right

		// Function 1-12
		
		else extraKeyInfo.ex = Normal;

		tickit_window_expose(bundle.typer, (TickitRect *) NULL);
	}

	return 1;
}

/**
 * @brief Any time the typer is exposed, this function will run. Typer will be
 * auto exposed at launch and any subsequent times from rootOnKey.
 * 
 * Works with bundle.typerBuffer and automatically mallocs it.
 * 
 * @param typer 
 * @param flags 
 * @param _info 
 * @param data 
 * @return int 
 */
static int typerOnExpose(TickitWindow *typer, TickitEventFlags flags, void *_info, void *data)
{
	TickitExposeEventInfo *info = _info;
	TickitRenderBuffer *rb = info->rb; // does not need to be unrefed
	TickitRect rect = info->rect;
	
	if (bundle.typerBuffer == NULL)
	{
		typerWidth = rect.cols - 2; // -2 because of extra gap from "> "
		bundle.typerBuffer = malloc(typerWidth * sizeof(char));
		
		// make empty
		for (int i = 0; i < typerWidth; ++i)
		{
			bundle.typerBuffer[i] = '\0';
		}
	}

	// Clear the window anew each time
	tickit_renderbuffer_goto(rb, 0, 0);
	tickit_renderbuffer_clear(rb);

	// should be unrefed
	TickitPen *p = tickit_pen_new();
	tickit_pen_set_colour_attr_desc(p, TICKIT_PEN_FG, "white");
	tickit_pen_set_bool_attr(p, TICKIT_PEN_BLINK, true);
	tickit_renderbuffer_setpen(rb, p);
	tickit_renderbuffer_text(rb, "> ");
	tickit_pen_set_bool_attr(p, TICKIT_PEN_BLINK, false);
	tickit_renderbuffer_setpen(rb, p);

	// Am I really using goto statements? Yes. 
	// 			--- Know thy enemy --- 
	// 
	// These could be extracted to functions in a typer header file
	// but ew I have so many global variables (that's bad enough itself)
	switch(extraKeyInfo.ex) 
	{
		case Backspace:
			goto backspace;
		default:
			goto normal;
	};


	backspace:
	{
		// avoids underflow
		if (numTyped != 0) numTyped--;

		bundle.typerBuffer[numTyped] = '\0';

		tickit_renderbuffer_textf(rb, "%s", bundle.typerBuffer);

		goto exit;
	}
	

	normal:
	{
		// First time running will auto expose typer and this avoids segfault
		if (lastKey.str == NULL) goto exit;
		

		/// @warning Issues abound if the thing you're writing to buffer is more than 1 char 
		if (numTyped < typerWidth)
		{
			strncat(bundle.typerBuffer, lastKey.str, 10);
			numTyped++;
		}

		tickit_renderbuffer_textf(rb, "%s", bundle.typerBuffer);
		goto exit;
	}


	exit:
	{
		tickit_pen_unref(p);
		return 1;
	}
}


static int dirDisplayOnExpose(TickitWindow *disDisplay, TickitEventFlags flags, void *_info, void *data)
{
	TickitExposeEventInfo *info = _info;
	TickitRenderBuffer *rb = info->rb; // does not need to be unrefed
	TickitRect rect = info->rect;

	DIR *d;
	struct dirent *dir;

	d = opendir(".");

	return 0;
}

/**
 * @brief Dereference or close items in bundle in reverse order
 * 
 */
static void empty_bundle(void)
{
	free(bundle.typerBuffer);
	tickit_window_close(bundle.dirDisplay);
	tickit_window_close(bundle.typer); 
	tickit_window_close(bundle.root);
	tickit_unref(bundle.t);
	
	// should I reset all the values to NULL?
	// I'm exiting right away anyway

	exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]) 
{
	tickit_debug_enabled = true;
    printf("Welcome to dirview. It's probably not complete yet.\n");

	bundle.t = (Tickit *) tickit_new_stdtty();

    if(bundle.t == NULL) 
	{
		fprintf(stderr, "Cannot create Tickit - %s\n", strerror(errno));
		return 1;
    }


	bundle.root = (TickitWindow *) tickit_get_rootwin(bundle.t);
	if(bundle.root == NULL) 
	{
		fprintf(stderr, "Cannot create root window - %s\n", strerror(errno));
		return 1;
  	}
	termRect = tickit_window_get_geometry(bundle.root);
	tickit_window_bind_event(bundle.root, TICKIT_WINDOW_ON_EXPOSE, (TickitBindFlags) 0, &rootOnExpose, NULL);


	bundle.typer = (TickitWindow *) tickit_window_new(bundle.root, (TickitRect){.top = termRect.lines-1, .left = 0, .lines = 2, .cols = termRect.cols}, (TickitWindowFlags) 0);
	if (bundle.typer == NULL) 
	{
		fprintf(stderr, "Cannot create typer window - %s\n", strerror(errno));
		return 1;
	} 
	// not currently working
	tickit_window_set_cursor_visible(bundle.typer, true);
	tickit_window_set_cursor_shape(bundle.typer, TICKIT_CURSORSHAPE_BLOCK);


	bundle.dirDisplay = (TickitWindow *) tickit_window_new(bundle.root, (TickitRect){.top=0, .cols=termRect.cols, .left=0, .lines = termRect.lines-2}, (TickitWindowFlags) 0);
	if (bundle.dirDisplay == NULL)
	{
		fprintf(stderr, "Cannot create dir display window - %s\n", strerror(errno));
		return 1;
	}


	tickit_window_bind_event(bundle.root, (TickitWindowEvent) TICKIT_WINDOW_ON_KEY, (TickitBindFlags) 0, &rootOnKey, NULL);
	tickit_window_bind_event(bundle.typer, (TickitWindowEvent) TICKIT_WINDOW_ON_EXPOSE, (TickitBindFlags) 0, &typerOnExpose, NULL);
	
	tickit_window_bind_event(bundle.dirDisplay, (TickitWindowEvent) TICKIT_WINDOW_ON_EXPOSE, (TickitBindFlags) 0, &dirDisplayOnExpose, NULL);

	tickit_window_expose(bundle.typer, NULL);

	
	

	tickit_run(bundle.t);

	// I don't think we could reach here?
	// Seems like only proper way to quit right now is Meta-Q
	empty_bundle();

    return EXIT_SUCCESS;
}