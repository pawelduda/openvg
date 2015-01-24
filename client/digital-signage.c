#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>
#include <termios.h>

#include "VG/openvg.h"
#include "VG/vgu.h"
#include "shapes.h"
#include "digital_signage.h"

// BEGIN data structs
struct NEWS_TICKER nt = {
	.pos_x = 20,
	.pos_y = 20,
	.screen_width = 1920,
	.font = &SansTypeface,
	.font_size = 20,
	.scroll_speed = 3, // 3 is the best balance between smoothness and readability
	.fade_speed = 0.05,
	.display_time = 4.0,

	.cur_pos_x = 20,
	.cur_msg_index = 0,
	.fades_in = true,
	.fades_out = false,
	.scrolls = false,
	.current_alpha = 0.,
	.msg_count = 4,
	.content = { 
		L"Przykładowa wiadomość 1", 
		L"Przykładowa wiadomość 2 Przykładowa wiadomość 2 Przykładowa wiadomość 2 Przykładowa wiadomość 2 Przykładowa wiadomość 2 Przykładowa wiadomość 2 Przykładowa wiadomość 2", 
		L"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean sit amet leo nec nunc pellentesque pellentesque vel et ante. Pellentesque sodales metus at mi.", 
		L"Quisque sagittis eros eu ipsum iaculis vehicula. Vivamus sodales eros odio, nec."
	}
};

struct PERSON_INFO person1 = {
	.name = L"Jan Kowalski",

	.text = {
		L"e-mail: example@domain.com",
		L"",
		L"Konsultacje: ",
		L"Środa: 12:00 - 14:00",
		L"Czwartek: 16:30 - 18:00",
		L"",
		L"Aliquam ac nisi turpis. Curabitur at tincidunt ligula, quis ornare lacus. ",
		L"Aliquam ac nisi turpis. Curabitur at tincidunt ligula, quis ornare lacus. ",
		L"In fringilla placerat luctus. Suspendisse potenti. Lorem ipsum dolor sit amet.",
	}
};

struct PERSON_INFO person2 = {
	.name = L"Tadeusz Brzęczyszczykiewicz",

	.text = {
		L"e-mail: example@domain.com",
		L"",
		L"Konsultacje: ",
		L"Środa: 12:00 - 14:00",
		L"Czwartek: 16:30 - 18:00",
		L"",
		L"In fringilla placerat luctus. Suspendisse potenti. Lorem ipsum dolor sit amet.",
		L"Aliquam ac nisi turpis. Curabitur at tincidunt ligula, quis ornare lacus. ",
		L"In fringilla placerat luctus. Suspendisse potenti. Lorem ipsum dolor sit amet.",
	}
};

struct PERSON_INFO person3 = {
	.name = L"Title Title Lorrrrrrem Ipsum",

	.text = {
		L"line1",
		L"line2",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem"
	}
};

struct PERSON_INFO person4 = {
	.name = L"Title Title Lorrrrrrem Ipsum",

	.text = {
		L"line1",
		L"line2",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem",
		L"line2 lorem ipsumlorem ipsumlorem ipsumlorem ipsumlorem ipsum lorem Lorrrrrremlorem"
	}
};

struct PEOPLE_LIST pl = {
	.display_time = 10.0,
	.fade_speed = 0.1,

	.fades_in = true,
	.fades_out = false,
	.current_alpha = 0.,
	.cur_index = 0,
	.start_pos_x = 110,
	.cur_pos_x = 110,

	.people_count = 4,
	.people = {
		&person1,
		&person2,
		&person3,
		&person4
	}
};
// END data structs

// BEGIN non-blocking and utility functions
void changemode(int dir) {
	static struct termios oldt, newt;

	if (dir == 1) {
		tcgetattr( STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~( ICANON | ECHO );
		tcsetattr( STDIN_FILENO, TCSANOW, &newt);
	} else {
		tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
	}	
}
 
int kbhit(void) {
	struct timeval tv;
	fd_set rdfs;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&rdfs);
	FD_SET(STDIN_FILENO, &rdfs);

	select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &rdfs);
}


double clamp(double d, double min, double max) {
	const double t = d < min ? min : d;
	return t > max ? max : t;
}
// END non-blocking and utility functions

// BEGIN common functions
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
// END common functions

// BEGIN newsticker specific functions
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
			
			if (nt->cur_msg_index < (nt->msg_count - 1)) {
				nt->cur_msg_index++;
			} else {
				nt->cur_msg_index = 0;
			}
			
			nt->fades_in = true;
		}
	}

	nt->current_alpha = Fade(nt->fades_in, nt->fades_out, nt->current_alpha, nt->fade_speed);
}
// END newsticker specific functions

// BEGIN person display specific functions
void UpdatePersonDisplay(struct PEOPLE_LIST *pl) {
	// fade in, then stop
	if (pl->fades_in) {
		pl->cur_pos_x += 1;

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
		pl->cur_pos_x += 1;

		if (IsFadeFinished(pl->fades_in, pl->fades_out, pl->current_alpha)) {
			pl->fades_in = false;
			pl->fades_out = false;
			
			if (pl->cur_index < (pl->people_count -1)) {
				pl->cur_index++;
			} else {
				pl->cur_index = 0;
			}

			pl->cur_pos_x = pl->start_pos_x + 20;
			pl->fades_in = true;
		}
	}

	pl->current_alpha = Fade(pl->fades_in, pl->fades_out, pl->current_alpha, pl->fade_speed);
}
// END people display specific functions
int main(int argc, char **argv) {
 	time_t rawtime;
 	struct tm *timeinfo;
 	char time_formatted[30];
	char key = 0;
	int counter_active_x, counter_cur_x, start_h, j, k;
	VGImage ath_logo;

	int width = 1920;
	int height = 1080;

	init(&width, &height);

	ath_logo = createImageFromJpeg("ath-logo.jpg");
	pl.people[0]->avatar = createImageFromJpeg("avatar1.jpg");
	pl.people[0]->qrcode = createImageFromJpeg("qrcode1.jpg");
	pl.people[1]->avatar = createImageFromJpeg("avatar2.jpg");
	pl.people[1]->qrcode = createImageFromJpeg("qrcode2.jpg");
	pl.people[2]->avatar = createImageFromJpeg("avatar3.jpg");
	pl.people[2]->qrcode = createImageFromJpeg("qrcode3.jpg");
	pl.people[3]->avatar = createImageFromJpeg("avatar4.jpg");
	pl.people[3]->qrcode = createImageFromJpeg("qrcode4.jpg");

	VGfloat stops[] = {
		0.5,  0.1,  0.2,  0.4, 1.0,
		0.8,  1.0,  1.0,  1.0, 1.0,
	};

	Start(width, height);

	changemode(1);
	while (!kbhit()) {
		Clear(1920, 1080);

		Stroke(0, 0, 0, 1);
		StrokeWidth(1);
		FillLinearGradient(0, 0, 2000, 0, stops, 2);
		Rect(-5, 880, 1930, 200);

		vgSetPixels(1550, 530, pl.people[pl.cur_index]->avatar, 0, 0, 300, 248);
		vgSetPixels(1550, 180, pl.people[pl.cur_index]->qrcode, 0, 0, 300, 300);
		vgSetPixels(1710, 910, ath_logo, 0, 0, 165, 131);

		Fill(21, 41, 82, 1);
		Rect(-5, 0, 1930, 60);

		StrokeWidth(0);
		Fill(30, 30, 30, 0.3);
		// Roundrect(70, 180, 1400, 600, 15, 15);
		Rect(70, 180, 1400, 600);

		counter_cur_x = 80;
		for (k = 0; k < pl.people_count; ++k) {
			Rect(counter_cur_x, 790, 25, 25);
			counter_cur_x += 35;
		}

		counter_cur_x = 15;
		for (k = 0; k < nt.msg_count; ++k) {
			Rect(counter_cur_x, 70, 25, 25);
			counter_cur_x += 35;
		}
		
		Fill(255, 255, 255, pl.current_alpha);

		WText(pl.cur_pos_x, 690, pl.people[pl.cur_index]->name, SansTypeface, 40);

		start_h = 620;
		for (j = 0; j < 10; ++j) {
			WText(pl.cur_pos_x, start_h, pl.people[pl.cur_index]->text[j], SansTypeface, 20);
			start_h -= 40;
		}

		Fill(255, 255, 255, 1);
		WText(50, 990, L"Przykładowy Nagłówek 1", SansTypeface, 40);
		WText(50, 930, L"Przykładowy Mniejszy Nagłówek 2", SansTypeface, 30);
	 	
		Fill(255, 255, 255, 1);
		Text(1555, 75, time_formatted, SansTypeface, 22);
		WText(1554, 160, L"Zeskanuj kod QR aby wysłać wiadomość", SansTypeface, 11);

	 	// people current rect
		Fill(255, 255, 255, pl.current_alpha);
		counter_active_x = 80 + 35 * pl.cur_index;
	 	Rect(counter_active_x, 790, 25, 25);

		// message current rect
		Fill(255, 255, 255, nt.current_alpha);
		counter_active_x = 15 + 35 * nt.cur_msg_index;
	 	Rect(counter_active_x, 70, 25, 25);

		WText(nt.cur_pos_x, 20, nt.content[nt.cur_msg_index], *nt.font, nt.font_size);
	 	time(&rawtime);
	 	timeinfo = localtime(&rawtime);
		strftime(time_formatted, 30, "%d.%m.%Y @ %H:%M:%S", timeinfo);

		UpdateNewsTicker(&nt);
		UpdatePersonDisplay(&pl);

		End(1920, 1080);
	}
 	key = getchar();
	printf("Got %c", key);
	changemode(0);
	


	finish(); 
	return 0;
}
