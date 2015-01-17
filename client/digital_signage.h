struct NEWS_TICKER {
        int pos_x;
        int pos_y;
        int screen_width;
        Fontinfo *font;
        int font_size;
        int scroll_speed;
        double fade_speed;
        int display_time;

        int cur_pos_x;
        int cur_msg_index;
        bool fades_in;
        bool fades_out;
        bool scrolls;
        double current_alpha;

        wchar_t content[3][200];
};
