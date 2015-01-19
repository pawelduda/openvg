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

#include "VG/openvg.h"
#include "VG/vgu.h"
#include "shapes.h"
#include "digital_signage.h"

struct NEWS_TICKER nt = {
	.pos_x = 20,
	.pos_y = 20,
	.screen_width = 1920,
	.font = &SansTypeface,
	.font_size = 20,
	.scroll_speed = 3,
	.fade_speed = 0.05,
	.display_time = 2,

	.cur_pos_x = 20,
	.cur_msg_index = 0,
	.fades_in = true,
	.fades_out = false,
	.scrolls = false,
	.current_alpha = 0,
	.content = { L"óśóśóśóśóśóśóśóśóśśóśóóśśóśóśóóśśóśółśąśóśóśóśóśóśóśóśóśóśśóśóóśśóśółśąś  ipsdasdasdumloremlorem óśóśóśóśóśóśóśóśóśśóśóóśśóśółśąś ipsdasdasdumlorem", L"testowa wiadomość óóó", L"ttt ipsdasdasdumloremlorem óśóśóśóśóśóśóśóśóśśóśóóśśóśółśąś" }
};

double clamp(double d, double min, double max) {
	const double t = d < min ? min : d;
	return t > max ? max : t;
}

void Fade(struct NEWS_TICKER *nt) {
	if (nt->fades_in && nt->current_alpha <= 1) {
		nt->current_alpha = clamp(nt->current_alpha + nt->fade_speed, 0.0, 1.0);
	} else if (nt->fades_out && nt->current_alpha >= 0) {
		nt->current_alpha = clamp(nt->current_alpha - nt->fade_speed, 0.0, 1.0);
	}
}

bool IsFadeFinished(struct NEWS_TICKER *nt) {
	if (nt->fades_in && nt->current_alpha >= 1) {
		return true;
	}

	if (nt->fades_out && nt->current_alpha <= 0) {
		return true;
	}

	return false;
}

VGfloat MeasureCurText(struct NEWS_TICKER *nt) {
	return WTextWidth(nt->content[nt->cur_msg_index], *nt->font, nt->font_size);
}

void StartFadeOut(struct NEWS_TICKER *nt) {
	nt->fades_out = true;
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
		if (IsFadeFinished(nt)) {
			nt->fades_in = false;
			nt->fades_out = false;
			// if scrollable, begin scrolling, else start countdown to fade out
			if (MeasureCurText(nt) > nt->screen_width) {
				nt->scrolls = true;
			} else {
				alarm(nt->display_time);
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

    // fade out, then stop
	if (nt->fades_out) {
		if (IsFadeFinished(nt)) {
			nt->fades_in = false;
			nt->fades_out = false;
			nt->cur_pos_x = nt->pos_x;
			
			if (nt->cur_msg_index < 2) {
				nt->cur_msg_index++;
			} else {
				nt->cur_msg_index = 0;
			}

			nt->fades_in = true;
		}
	}

	Fade(nt);


}

void ALARMhandler(int sig) {
	signal(SIGALRM, SIG_IGN);
	StartFadeOut(&nt);
	signal(SIGALRM, ALARMhandler);
}

int main(int argc, char **argv) {

 	signal(SIGALRM, ALARMhandler);
 	time_t rawtime;
 	struct tm *timeinfo;
 	char time_formatted[30];

	int width, height, circleX;
	VGImage img1, bg, avatar;

	width = 1920;
	height = 1080;

	init(&width, &height);
	Start(width, height);

	bg = createImageFromJpeg("bg.jpg");
	avatar = createImageFromJpeg("avatar-placeholder.jpg");

	while (1) {
		vgSetPixels(0, 0, bg, 0, 0, 1920, 1080);
		vgSetPixels(70, 150, avatar, 0, 0, 300, 248);
		vgSetPixels(70, 550, avatar, 0, 0, 300, 248);
		vgSetPixels(950, 150, avatar, 0, 0, 300, 248);
		vgSetPixels(950, 550, avatar, 0, 0, 300, 248);

		Fill(21, 41, 82, 1);
		Stroke(0, 0, 0, 1);
		StrokeWidth(2);
		Rect(-5, 880, 1930, 200);
		Rect(-5, 0, 1930, 60);
		
		Fill(255, 255, 255, 1);
		WText(410, 350, L"First name last name", SansTypeface, 24);
		WText(410, 300, L"Sample text:", SansTypeface, 20);
		WText(410, 260, L"Śr 15:30-17:30", SansTypeface, 20);
		WText(410, 220, L"Czw 12:00-15:00", SansTypeface, 20);

		WText(410, 750, L"First name last name", SansTypeface, 24);
		WText(410, 700, L"Sample text:", SansTypeface, 20);
		WText(410, 660, L"Śr 15:30-17:30", SansTypeface, 20);
		WText(410, 620, L"Czw 12:00-15:00", SansTypeface, 20);

		WText(1290, 350, L"First name last name", SansTypeface, 24);
		WText(1290, 300, L"Sample text:", SansTypeface, 20);
		WText(1290, 260, L"Śr 15:30-17:30", SansTypeface, 20);
		WText(1290, 220, L"Czw 12:00-15:00", SansTypeface, 20);

		WText(1290, 750, L"First name last name", SansTypeface, 24);
		WText(1290, 700, L"Sample text:", SansTypeface, 20);
		WText(1290, 660, L"Śr 15:30-17:30", SansTypeface, 20);
		WText(1290, 620, L"Czw 12:00-15:00", SansTypeface, 20);

		Fill(255, 255, 255, 1);
		WText(50, 990, L"Sample header", SansTypeface, 40);
		WText(50, 930, L"Sample header sample header", SansTypeface, 30);

		Text(1490, 1000, time_formatted, SansTypeface, 24);
	 	
		Fill(21, 41, 82, 1);
	 	
	 	Circle(914, 85, 25);
	 	Circle(953, 85, 25);
	 	Circle(992, 85, 25);

		Fill(255, 255, 255, nt.current_alpha);
	 	
		circleX = 914 + 39 * nt.cur_msg_index;

		StrokeWidth(0);

	 	Circle(circleX, 85, 21);

		WText(nt.cur_pos_x, 20, nt.content[nt.cur_msg_index], *nt.font, nt.font_size);

	 	time(&rawtime);
	 	timeinfo = localtime(&rawtime);
		strftime(time_formatted, 30, "%d.%m.%Y @ %H:%M:%S", timeinfo);

		UpdateNewsTicker(&nt);

		End();
	}
}
