/* This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2013-2014 Kevin Lange
 */
#include <math.h>
#include <stdio.h>
#include <cairo.h>

 

 

void render() {
//	draw_fill(ctx, rgba(0,0,0,127));

	int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, 1268);
	cairo_surface_t * surface = cairo_image_surface_create_for_data(0xfd000000, CAIRO_FORMAT_ARGB32, 1268, 1024, stride);
	cairo_t * cr = cairo_create(surface);

	cairo_set_line_width (cr, 6);

	cairo_rectangle (cr, 12, 12, 232, 70);
	cairo_new_sub_path (cr); cairo_arc (cr, 64, 64, 40, 0, 2*M_PI);
	cairo_new_sub_path (cr); cairo_arc_negative (cr, 192, 64, 40, 0, -2*M_PI);

	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
	cairo_set_source_rgb (cr, 0, 0.7, 0); cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0, 0, 0); cairo_stroke (cr);

	cairo_translate (cr, 0, 128);
	cairo_rectangle (cr, 12, 12, 232, 70);
	cairo_new_sub_path (cr); cairo_arc (cr, 64, 64, 40, 0, 2*M_PI);
	cairo_new_sub_path (cr); cairo_arc_negative (cr, 192, 64, 40, 0, -2*M_PI);

	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_WINDING);
	cairo_set_source_rgb (cr, 0, 0, 0.9); cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0, 0, 0); cairo_stroke (cr);

	cairo_surface_flush(surface);
	cairo_destroy(cr);
	cairo_surface_flush(surface);
	cairo_surface_destroy(surface);

//	yutani_flip(yctx, window);
}
int main()
{
	render();

}
