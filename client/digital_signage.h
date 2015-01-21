struct NEWS_TICKER {
        int pos_x;
        int pos_y;
        int screen_width;
        Fontinfo *font;
        int font_size;
        int scroll_speed;
        double fade_speed;
        double display_time;

        int cur_pos_x;
        int cur_msg_index;
        bool fades_in;
        bool fades_out;
        bool scrolls;
        double current_alpha;
        time_t start_i;
        time_t cur_i;

        wchar_t content[4][200];
};

struct PERSON_INFO {
        char photo_filename[50];
        char qrcode_filename[50];
        wchar_t name[100];
        wchar_t text[10][200];
};

struct PEOPLE_LIST {
        int cur_index;
        double display_time;
        double fade_speed;

        struct PERSON_INFO *people[4];

        bool fades_in;
        bool fades_out;
        double current_alpha;
        time_t start_i;
        time_t cur_i;
};
