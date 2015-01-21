#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>

#include "VG/openvg.h"
#include "VG/vgu.h"
#include "shapes.h"
#include "digital_signage.h"

double clamp(double d, double min, double max) {
	const double t = d < min ? min : d;
	return t > max ? max : t;
}

struct NEWS_TICKER nt = {
	.pos_x = 20,
	.pos_y = 20,
	.screen_width = 1920,
	.font = &SansTypeface,
	.font_size = 20,
	.scroll_speed = 2, // 3 is the best balance between smoothness and readability
	.fade_speed = 0.05,
	.display_time = 4.0,

	.cur_pos_x = 20,
	.cur_msg_index = 0,
	.fades_in = true,
	.fades_out = false,
	.scrolls = false,
	.current_alpha = 0.,
	.content = { 
		L"tralalalalaalaalllalal", 
		L"óśóśóśóśóśóśóśóśóśśóśóóśśóśóśóóśśóśółśąśóśóśóśóśóśóśóśóśóśśóśóóśśóśółśąś  ipsdasdasdumloremlorem óśóśóśóśóśóśóśóśóśśóśóóśśóśółśąś ipsdasdasdumlorem", 
		L"testowa wiadomość óóó", 
		L"ttt ipsdasdasdumloremlorem óśóśóśóśóśóśóśóśóśśóśóóśśóśółśąś" 
	}
};

struct PERSON_INFO person1 = {
	.photo_filename = "avatar-placeholder.jpg",
	.qrcode_filename = "qrcode.jpg",
	.name = L"Title Title",

	.text = {
		L"line1",
		L"line2"
	}
};

struct PERSON_INFO person2 = {
	.photo_filename = "avatar-placeholder.jpg",
	.qrcode_filename = "qrcode.jpg",
	.name = L"Title Title Lorrrrrrem Ipsum",
	.text = {
		L"line1",
		L"line2"
	}
};

struct PEOPLE_LIST people_list = {
	.display_time = 5.0,
	.fade_speed = 0.05,

	.fades_in = true,
	.fades_out = false,
	.current_alpha = 0.,
	.cur_index = 0,

	.people = {
		&person1,
		&person2
	}
};

// Common functions
double Fade(bool fades_in, bool fades_out, double current_alpha, double fade_speed) {
	if (fades_in && current_alpha <= 1) {
		return clamp(current_alpha + fade_speed, 0.0, 1.0);
	} else if (fades_out && current_alpha >= 0) {
		return clamp(current_alpha - fade_speed, 0.0, 1.0);
	}

	return 1.0;
}

bool IsFadeFinished(bool fades_in, bool fades_out, double current_alpha) {
	if (fades_in && current_alpha >= 1) {
		return true;
	}

	if (fades_out && current_alpha <= 0) {
		return true;
	}

	return false;
}

// NewsTicker specific functions
VGfloat MeasureCurText(struct NEWS_TICKER *nt) {
	return WTextWidth(nt->content[nt->cur_msg_index], *nt->font, nt->font_size);
}

void ScrollText(struct NEWS_TICKER *nt) {
	if (!nt->fades_out && !nt->fades_in) {
		if (MeasureCurText(nt) > nt->screen_width) {
			nt->cur_pos_x -= nt->scroll_speed;
		}
	}
}

bool IsScrollFinished(struct NEWS_TICKER *nt) {
	if ((nt->cur_pos_x + MeasureCurText(nt)) <= (nt->screen_width - 25)) {
		return true;
	}

	return false;
}

void UpdateNewsTicker(struct NEWS_TICKER *nt) {
	// fade in, then stop
	if (nt->fades_in) {
		if (IsFadeFinished(nt->fades_in, nt->fades_out, nt->current_alpha)) {
			nt->fades_in = false;
			nt->fades_out = false;
			// if scrollable, begin scrolling, else start countdown to fade out
			if (MeasureCurText(nt) > nt->screen_width) {
				nt->scrolls = true;
			} else {
				time(&nt->start_i);
			}
		}
	}

    // if scrollable, scroll until end, then fade out
	if (nt->scrolls && (!nt->fades_in && !nt->fades_out)) {
		if (!IsScrollFinished(nt)) {
			ScrollText(nt);
		} else {
			nt->scrolls = false;
			nt->fades_out = true;
		}
	}	

	if (!nt->scrolls && !nt->fades_in && !nt->fades_out) {
		time(&nt->cur_i);
		if (difftime(nt->cur_i, nt->start_i) > nt->display_time) {
			nt->fades_out = true;
			time(&nt->start_i);
		}
	}

    // fade out, then stop
	if (nt->fades_out) {
		if (IsFadeFinished(nt->fades_in, nt->fades_out, nt->current_alpha)) {
			nt->fades_in = false;
			nt->fades_out = false;
			nt->cur_pos_x = nt->pos_x;
			
			if (nt->cur_msg_index < 3) {
				nt->cur_msg_index++;
			} else {
				nt->cur_msg_index = 0;
			}
			
			nt->fades_in = true;
		}
	}

	nt->current_alpha = Fade(nt->fades_in, nt->fades_out, nt->current_alpha, nt->fade_speed);
}

void UpdatePersonDisplay(struct PEOPLE_LIST *pl) {
	// fade in, then stop
	if (pl->fades_in) {
		if (IsFadeFinished(pl->fades_in, pl->fades_out, pl->current_alpha)) {
			pl->fades_in = false;
			pl->fades_out = false;

			time(&pl->start_i);
		}
	}

	if (!pl->fades_in && !pl->fades_out) {
		time(&pl->cur_i);
		if (difftime(pl->cur_i, pl->start_i) > pl->display_time) {
			pl->fades_out = true;
			time(&pl->start_i);
		}
	}

	    // fade out, then stop
	if (pl->fades_out) {
		if (IsFadeFinished(pl->fades_in, pl->fades_out, pl->current_alpha)) {
			pl->fades_in = false;
			pl->fades_out = false;
			
			if (pl->cur_index < 1) {
				pl->cur_index++;
			} else {
				pl->cur_index = 0;
			}

			pl->fades_in = true;
		}
	}

	pl->current_alpha = Fade(pl->fades_in, pl->fades_out, pl->current_alpha, pl->fade_speed);
}


int main(int argc, char **argv) {
 	time_t rawtime;
 	struct tm *timeinfo;
 	char time_formatted[30];

	int width, height, circleX;
	VGImage bg, qr, avatar, ath_logo;

	width = 1920;
	height = 1080;

	init(&width, &height);

	bg = createImageFromJpeg("bg.jpg");
	qr = createImageFromJpeg("qrcode.jpg");
	ath_logo = createImageFromJpeg("ath-logo.jpg");
	avatar = createImageFromJpeg("avatar-placeholder.jpg");

	VGfloat stops[] = {
		0.2,  0.1,  0.2,  0.4, 1.0,
		0.8,  1.0,  1.0,  1.0, 1.0,
	};

	Start(width, height);

	while (1) {
		Clear(1920, 1080);

		Stroke(0, 0, 0, 1);
		StrokeWidth(1);
		FillLinearGradient(0, 0, 2000, 0, stops, 2);
		Rect(-5, 880, 1930, 200);

		// vgSetPixels(0, 0, bg, 0, 0, 1920, 880);
		vgSetPixels(1550, 550, avatar, 0, 0, 300, 248);
		vgSetPixels(1550, 200, qr, 0, 0, 300, 300);
		vgSetPixels(1710, 910, ath_logo, 0, 0, 165, 131);

		Fill(21, 41, 82, 1);
		Rect(-5, 0, 1930, 60);
		
		Fill(255, 255, 255, people_list.current_alpha);

		WText(70, 750, people_list.people[people_list.cur_index]->name, SansTypeface, 40);
		WText(70, 700, L"Sample text:", SansTypeface, 20);
		WText(70, 660, L"Śr 15:30-17:30", SansTypeface, 20);
		WText(70, 620, L"Czw 12:00-15:00", SansTypeface, 20);

		Fill(255, 255, 255, 1);
		WText(50, 990, L"Sample Header", SansTypeface, 40);
		WText(50, 930, L"Sample Header Sample Header", SansTypeface, 30);
	 	
		Fill(21, 41, 82, 1);
	 	
	 	Rect(15, 70, 25, 25);
	 	Rect(50, 70, 25, 25);
	 	Rect(85, 70, 25, 25);

		Fill(255, 255, 255, 1);
		Text(1555, 75, time_formatted, SansTypeface, 22);

		Fill(255, 255, 255, nt.current_alpha);
		circleX = 15 + 35 * nt.cur_msg_index;

	 	Rect(circleX, 70, 25, 25);

		WText(nt.cur_pos_x, 20, nt.content[nt.cur_msg_index], *nt.font, nt.font_size);
	 	time(&rawtime);
	 	timeinfo = localtime(&rawtime);
		strftime(time_formatted, 30, "%d.%m.%Y @ %H:%M:%S", timeinfo);

		UpdateNewsTicker(&nt);
		UpdatePersonDisplay(&people_list);

		End(1920, 1080);
	}
}
