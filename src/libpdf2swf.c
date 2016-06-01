#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include "../config.h"
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "../lib/os.h"
#include "../lib/rfxswf.h"
#include "../lib/devices/swf.h"
#include "../lib/devices/polyops.h"
#include "../lib/devices/record.h"
#include "../lib/devices/rescale.h"
#include "../lib/gfxfilter.h"
#include "../lib/pdf/pdf.h"

#include "libpdf2swf.h"
#define SWFDIR concatPaths(getInstallationPath(), "swfs")

static gfxsource_t*driver = 0;
static gfxdevice_t*out = 0;

static int maxwidth = 0, maxheight = 0;

static char * pagerange = 0;
static char * password = 0;

static int zlib = 0;

static char * preloader = 0;
static char * viewer = 0;
static int xnup = 1;
static int ynup = 1;

static int info_only = 0;
static int max_time = 0;
static int flatten = 0;

char* fontpaths[256];
int fontpathpos = 0;

int move_x = 0;
int move_y = 0;
int custom_move = 0;
int clip_x1 = 0, clip_y1 = 0, clip_x2 = 0, clip_y2 = 0;
int custom_clip = 0;

static int system_quiet = 0;
static gfxdevice_t swf, wrap, rescale;

gfxdevice_t* create_output_device()
{
	gfxdevice_swf_init(&swf);

	out = &swf;
	if (flatten) {
		gfxdevice_removeclippings_init(&wrap, &swf);
		out = &wrap;
	}

	if (maxwidth || maxheight) {
		gfxdevice_rescale_init(&rescale, out, maxwidth, maxheight, 0);
		out = &rescale;
	}

	return out;
}

ErrorCode convert(const char* filename, int page, const char* outputname,const char* password)
{
	int ret;
	char buf[256];
	int numfonts = 0;
	int t;
	char t1searchpath[1024];
	int nup_pos = 0;
	int x, y;
	int one_file_per_page = 0;


	driver = gfxsource_pdf_create();
	if (!filename || !outputname)
	{
		return PARAM_ERROR;
	}


	//is_in_range(0x7fffffff, pagerange);

	char fullname[256];
	if (password && *password) {
		sprintf(fullname, "%s|%s", filename, password);
		filename = fullname;
	}

	if (pagerange)
		driver->setparameter(driver, "pages", pagerange);
	
	char*u = 0;
	if ((u = strchr(outputname, '%'))) {
		if (strchr(u + 1, '%') ||
			strchr(outputname, '%') != u) {
			return PARAM_ERROR;
		}

		one_file_per_page = 1;
		char*pattern = (char*)malloc(strlen(outputname) + 2);

		int l = u - outputname + 1;
		memcpy(pattern, outputname, l);
		pattern[l] = 'd';
		strcpy(pattern + l + 1, outputname + l);
		outputname = pattern;
	}

	gfxdocument_t* pdf = driver->open(driver, filename);
	if (!pdf) {
		return FILE_OPEN_ERROR;
	}

	struct mypage_t {
		int x;
		int y;
		gfxpage_t*page;
	} pages[4];

	int pagenum = 0;
	int frame = 1;
	int pagenr;

	for (pagenr = 1; pagenr <= pdf->num_pages; pagenr++)
	{
		//if (is_in_range(pagenr, pagerange)) {}
		char mapping[80];
		sprintf(mapping, "%d:%d", pagenr, frame);
		pdf->setparameter(pdf, "pagemap", mapping);
		pagenum++;
		if (pagenum == xnup*ynup || (pagenr == pdf->num_pages && pagenum > 1)) {
			pagenum = 0;
			frame++;
		}
	}

	if (pagerange && !pagenum && frame == 1) {
		return RANGE_ERROR;
	}

	pagenum = 0;

	gfxdevice_t*out = create_output_device();
	pdf->prepare(pdf, out);

	for (pagenr = 1; pagenr <= pdf->num_pages; pagenr++)
	{
		//if (is_in_range(pagenr, pagerange)) {}
		gfxpage_t* page = pages[pagenum].page = pdf->getpage(pdf, pagenr);
		pages[pagenum].x = 0;
		pages[pagenum].y = 0;
		pages[pagenum].page = page;
		pagenum++;

		if (pagenum == xnup*ynup || (pagenr == pdf->num_pages && pagenum > 1)) {

			int t;
			int xmax[xnup], ymax[xnup];
			int x, y;
			int width = 0, height = 0;

			memset(xmax, 0, xnup*sizeof(int));
			memset(ymax, 0, ynup*sizeof(int));

			for (y = 0; y < ynup; y++) {
				for (x = 0; x<xnup; x++) {
					int t = y*xnup + x;

					if (pages[t].page->width > xmax[x])
						xmax[x] = (int)pages[t].page->width;
					if (pages[t].page->height > ymax[y])
						ymax[y] = (int)pages[t].page->height;
				}
			}

			for (x = 0; x < xnup; x++) {
				width += xmax[x];
				xmax[x] = width;
			}

			for (y = 0; y < ynup; y++) {
				height += ymax[y];
				ymax[y] = height;
			}

			if (custom_clip) {
				out->startpage(out, clip_x2 - clip_x1, clip_y2 - clip_y1);
			}
			else {
				out->startpage(out, width, height);
			}

			for (t = 0; t < pagenum; t++) {
				int x = t%xnup;
				int y = t / xnup;
				int xpos = x > 0 ? xmax[x - 1] : 0;
				int ypos = y > 0 ? ymax[y - 1] : 0;

				pages[t].page->rendersection(pages[t].page, out, custom_move ? move_x : xpos,
					custom_move ? move_y : ypos,
					custom_clip ? clip_x1 : 0 + xpos,
					custom_clip ? clip_y1 : 0 + ypos,
					custom_clip ? clip_x2 : pages[t].page->width + xpos,
					custom_clip ? clip_y2 : pages[t].page->height + ypos);
			}
			out->endpage(out);
			for (t = 0; t < pagenum; t++) {
				pages[t].page->destroy(pages[t].page);
			}
			pagenum = 0;

			if (one_file_per_page) {
				gfxresult_t*result = out->finish(out); out = 0;
				char buf[1024];
				sprintf(buf, outputname, pagenr);
				if (result->save(result, buf) < 0) {
					return 1;
				}
				result->destroy(result); result = 0;
				out = create_output_device();
				pdf->prepare(pdf, out);
			}
		}
	}

	if (one_file_per_page) {
		gfxresult_t*result = out->finish(out); out = 0;
		result->destroy(result); result = 0;
	}
	else {
		gfxresult_t*result = out->finish(out);
		if (result->save(result, outputname) < 0) {
			return SAVE_ERROR;
		}
		result->destroy(result); result = 0;
	}

	pdf->destroy(pdf);
	driver->destroy(driver);
	return NOERROR;
}

