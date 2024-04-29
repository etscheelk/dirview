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

/**
 * @brief Everything in bundle is the stuff that should be freed or unref'd
 * at end of program. Dereference or close in reverse order.
 */
struct bundle
{
	//
	Tickit *t;

	//
	TickitWindow *root;

	//
	TickitWindow *typer;

	// Buffer for the text we store in the typer. Size determined by 2*cols.
	// Declared first in typerOnExpose.
	// Size is cols for flexibility on window resize (not implemented currently)
	char *typerBuffer;
} bundle = {.t = NULL, .root = NULL, .typer = NULL, .typerBuffer = NULL};

TickitPenRGB8 blue = {.r = 10, .g = 100, .b = 200};

TickitRect termRect;
unsigned short numTyped = 0;
TickitKeyEventInfo lastKey;

// Function Declarations
// Events
static int rootOnExpose	(TickitWindow *win, TickitEventFlags flags, void *_info, void *data);
static int rootOnKey	(TickitWindow *win, TickitEventFlags flags, void *_info, void *data);
static int typerOnExpose(TickitWindow *win, TickitEventFlags flags, void *_info, void *data);

// Other
void empty_bundle(void);

/**
 * If I want to do fuzzy searching on local files, CLI tool fzf
 * 
 */



// Never seen struct created like this before
// struct
// {
// 	char *name;
// 	int val;
//
// 	TickitPen *pen_fg, *pen_fg_hi, *pen_bg, *pen_bg_hi;
// } penColors[] = 
// {
// 	{"blue", 1},
// 	{"red", 1}
// };


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
		return 1;
	}

	// if user types Meta-q or Meta-Q, quit the prorgram
	if (info->mod == TICKIT_MOD_ALT && (streq(info->str, "M-q") || streq(info->str, "M-Q")))
	{
		empty_bundle();
	}

	
	if (!(info->mod & TICKIT_MOD_ALT) && !(info->mod & TICKIT_MOD_CTRL)) 
	{
		tickit_window_expose(bundle.typer, (TickitRect*) NULL);
	}

	return 1;
}

static int typerOnExpose(TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
	TickitExposeEventInfo *info = _info;

	TickitRenderBuffer *rb = info->rb;
	TickitRect rect = info->rect;
	
	if (bundle.typerBuffer == NULL)
	{
		printf("malloced typer buffer\n");
		bundle.typerBuffer = malloc(rect.cols * sizeof(char));
	}

	// First time running will auto expose typer and this avoids segfault
	if (lastKey.str == NULL) return 0;

	strncat(bundle.typerBuffer, lastKey.str, 1);

	// Clear the window anew each time
	tickit_renderbuffer_goto(rb, 0, 0);
	tickit_renderbuffer_clear(rb);

	TickitPen *p = tickit_pen_new();
	tickit_pen_set_colour_attr_desc(p, TICKIT_PEN_FG, "white");
	tickit_pen_set_bool_attr(p, TICKIT_PEN_BLINK, true);
	tickit_renderbuffer_setpen(rb, p);
	tickit_renderbuffer_text(rb, "> ");

	// printf("\nlen: %d\n", strnlen(lastKey.str, 64));

	// strncat(typerBuffer, lastKey.str, 5);

	tickit_pen_set_bool_attr(p, TICKIT_PEN_BLINK, false);
	tickit_renderbuffer_setpen(rb, p);

	// tickit_renderbuffer_text(rb, "search_term_:)");

	// tickit_renderbuffer_text(rb, "\b");

	// tickit_renderbuffer_textf(rb, "%s", typerBuffer);

	// printf("\n");
	// tickit_renderbuffer_text(rb, lastKey.str);
	tickit_renderbuffer_textf(rb, "%s", bundle.typerBuffer);
	// tickit_renderbuffer_char(rb, *lastKey.str);
	// tickit_renderbuffer_text(rb, typerBuffer);
	// tickit_renderbuffer_textf(rb, "%s", typerBuffer);

	// int i = 0;
	// while (*(typerBuffer + i) != '\0' )
	// {
	// 	printf(" %c", *(typerBuffer + i));
	// 	i++;
	// }

	return 1;
}

// Free the elements of bundle in reverse order
void empty_bundle(void)
{
	free(bundle.typerBuffer);
	tickit_window_close(bundle.typer); 
	tickit_window_close(bundle.root);
	tickit_unref(bundle.t);
	
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
	
	// check suspend is CTRL Z signal
	// tickit_window_bind_event(root, TICKIT_WINDOW_ON_KEY, (TickitBindFlags) 0, &checksuspend, NULL);

	bundle.typer = (TickitWindow *) tickit_window_new(bundle.root, (TickitRect){.top = termRect.lines-1, .left = 0, .lines = 2, .cols = termRect.cols}, (TickitWindowFlags) 0);
	if (bundle.typer == NULL) 
	{
		fprintf(stderr, "Cannot create typer window - %s\n", strerror(errno));
		return 1;
	} 

	tickit_window_bind_event(bundle.root, (TickitWindowEvent) TICKIT_WINDOW_ON_KEY, (TickitBindFlags) 0, &rootOnKey, NULL);
	tickit_window_bind_event(bundle.typer, (TickitWindowEvent) TICKIT_WINDOW_ON_EXPOSE, (TickitBindFlags) 0, &typerOnExpose, NULL);
	tickit_window_expose(bundle.typer, NULL);

	tickit_run(bundle.t);

	// I don't think we could reach here?
	// Seems like only proper way to quit right now is Meta-Q
	empty_bundle();

    return EXIT_SUCCESS;
}
