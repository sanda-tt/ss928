/*
 * Copyright (c) 2025 UncleRoderick@hotmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef XVP_COMMON_OSD_H
#define XVP_COMMON_OSD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    int factor;
    unsigned int font_color;
    unsigned int border_color;
    int stride;
    int width;
    int height;
} xvp_osd_info;

const char *xvp_common_osd_get_version(void);
int xvp_common_osd_init(char *hzk_file);
void xvp_common_osd_exit(void);
void xvp_common_osd_draw_text(unsigned char *bitmap_data, unsigned char *text_string,
        int offset_x, int offset_y, xvp_osd_info *osd_info);
int xvp_common_osd_draw_poit(unsigned char *bitmap_data, int start_x, int start_y, xvp_osd_info *osd_info);
int xvp_common_osd_draw_line(unsigned char *bitmap_data, int start_x, int start_y, int end_x, int end_y,
         unsigned char border_width, xvp_osd_info *osd_info);
int xvp_common_osd_draw_rect(unsigned char *bitmap_data,  int start_x, int start_y, int end_x, int end_y,
        unsigned char border_width, xvp_osd_info *osd_info);
int xvp_common_osd_parse_string(char *str, int *max_len, int *line_cnt);
int xvp_common_osd_calculate_canvas_size(char *str, int factor, int *width, int *height);
int xvp_common_osd_string_filter(char *src, char *dst, int dst_len, int max_line_num, int max_line_len);
int xvp_common_osd_utf8_to_gb2312(const unsigned char *in_buf, int in_len, unsigned char *out_buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
