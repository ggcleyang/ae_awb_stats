//
// Created by ly on 2022/3/10.
//

#include "ae_awb_stats.h"


void calc_grid_info(uint16 start_pos,uint16 h_w,uint16 zone_num,uint16* grid_info){

    uint16 integer,remainder;
    integer   = h_w / DIV_0_TO_1(zone_num);
    remainder = h_w % DIV_0_TO_1(zone_num);
    grid_info[0] = start_pos;
    for(uint16 i = 1;i < zone_num; i++){
        if (remainder > 0)
        {
            grid_info[i] = grid_info[i - 1] + integer + 1;
            remainder = remainder - 1;
        }
        else
        {
            grid_info[i] = grid_info[i - 1] + integer ;
        }
    }
    return;
}
uint16 get_striping_img_start(striping_mode_cfg stripingModeCfg){

    uint16  StripingIndex = stripingModeCfg.u16StripingIndex;
    uint16  *StripingStartX = stripingModeCfg.u16StripingStartX;
    //uint16  *StripingWidth = stripingModeCfg.u16StripingWidth;
    uint16  OverLap_n = stripingModeCfg.u16OverLap;

    uint16 striping_img_start;
    if(0 == StripingIndex){
        striping_img_start = StripingStartX[0];
    }
    else{
        striping_img_start = StripingStartX[StripingIndex] + OverLap_n;
    }
    return striping_img_start;
}
uint16 get_striping_img_width(striping_mode_cfg stripingModeCfg){
    //uint16  StripingNum = stripingModeCfg.u16StripingNum;
    uint16  StripingIndex = stripingModeCfg.u16StripingIndex;
    uint16  *StripingStartX = stripingModeCfg.u16StripingStartX;
    uint16  *StripingWidth = stripingModeCfg.u16StripingWidth;
    uint16  OverLap_n = stripingModeCfg.u16OverLap;
    uint16 striping_img_width;
    if(0 == StripingIndex ){
        striping_img_width = StripingWidth[0] - OverLap_n;
    }
    else if((STRIPINGNUM-1) == StripingIndex){
        striping_img_width = StripingWidth[STRIPINGNUM-1] - OverLap_n;
    }
    else{
        striping_img_width = StripingWidth[StripingIndex] - 2*OverLap_n;
    }
    return striping_img_width;
}
uint16 get_striping_gridX_info(uint16* grid_info,uint16 block_num,striping_mode_cfg stripingModeCfg){
    uint16 striping_img_start;
    uint16 striping_img_width;
    uint16 div_num;
    uint16 index = 0;
    for(uint8 sn = 0;sn < STRIPINGNUM; sn++){
        stripingModeCfg.u16StripingIndex = sn;
        striping_img_start = get_striping_img_start(stripingModeCfg);
        striping_img_width = get_striping_img_width(stripingModeCfg);
        if(sn < block_num % DIV_0_TO_1(STRIPINGNUM)){
            div_num = block_num / DIV_0_TO_1(STRIPINGNUM) + 1;
        }
        else{
            div_num = block_num / DIV_0_TO_1(STRIPINGNUM);
        }
        calc_grid_info( striping_img_start, striping_img_width, div_num,&(grid_info[index]));
        index = index + div_num;
    }
    return 0;
}
void isp_awb_stats(uint16*img,img_info raw_info,striping_mode_cfg stripingModeCfg, awb_stats *awbStats,const awb_stats_cfg awbStatsCfg,awb_grid_info *awbGridInfo){

    uint16 img_height = raw_info.u16ImgHeight;
    uint16 img_width = raw_info.u16ImgWidth;
    uint8 bayer_pattern = raw_info.u8bayermode;
    uint16 start_x = 0;
    uint16 start_y = 0;

    //uint16 striping_img_width;
    //uint16 striping_img_start;

    uint16 r = 0,g =0,b=0;
    uint8 y;
    uint16 r2g = 0,b2g =0;
    uint8 y_l = awbStatsCfg.y_l;
    uint8 y_h = awbStatsCfg.y_h;
    uint16 r2g_l = awbStatsCfg.r2g_l;
    uint16 r2g_h = awbStatsCfg.r2g_h;
    uint16 b2g_l = awbStatsCfg.b2g_l;
    uint16 b2g_h = awbStatsCfg.b2g_h;
    uint32 countAll = 0;
    uint64 sum_r2g =0,sum_b2g =0,sum_r = 0,sum_g =0,sum_b = 0;
    if(IS_STRIPING_MODE){

    }
    else{


        uint16 zone_width = img_width/AE_AWB_ZONE_COLUMN;
        uint16 zone_height = img_height/AE_AWB_ZONE_ROW;
        uint16 x_offset = (img_width%AE_AWB_ZONE_COLUMN)/2;
        uint16 y_offset = (img_height%AE_AWB_ZONE_ROW)/2;

        for(uint16 col=0;col<AE_AWB_ZONE_COLUMN+1;col++){
            awbGridInfo->au16GridXPos[col] = x_offset + zone_width*col;
        }
        for(uint16 row=0;row<AE_AWB_ZONE_ROW+1;row++){
            awbGridInfo->au16GridYPos[row] = y_offset + zone_height*row;
        }

        for (uint16 y1 = 0; y1 < img_height; y1=y1+2) {
            for (uint16 x1 = 0; x1 < img_width; x1=x1+2) {

               y = (img[y1*img_width+x1]+ img[y1*img_width+x1+1] + img[(y1+1)*img_width+x1] +img[(y1+1)*img_width+x1+1])>>6;
                if(BPRG == bayer_pattern){
                    r = img[y1*img_width+x1];
                    g = (img[y1*img_width+x1+1] + img[(y1+1)*img_width+x1])>>1;
                    b = img[(y1+1)*img_width+x1+1];
                    r2g = (r<<8)/g;
                    b2g = (b<<8)/g;
                }
                else if(BPGR == bayer_pattern){
                    r = img[y1*img_width+x1+1];
                    g = (img[y1*img_width+x1] + img[(y1+1)*img_width+x1+1])>>1;
                    b = img[(y1+1)*img_width+x1];
                    r2g = (r<<8)/g;
                    b2g = (b<<8)/g;

                }
                else if(BPGB == bayer_pattern){
                    r = img[(y1+1)*img_width+x1];
                    g = (img[y1*img_width+x1] + img[(y1+1)*img_width+x1+1])>>1;
                    b = img[y1*img_width+x1];
                    r2g = (r<<8)/g;
                    b2g = (b<<8)/g;
                }
                //if(BPBG == bayer_pattern)
                else {
                    r = img[(y1+1)*img_width+x1+1];
                    g = (img[y1*img_width+x1+1] + img[(y1+1)*img_width+x1])>>1;
                    b = img[y1*img_width+x1];
                    r2g = (r<<8)/g;
                    b2g = (b<<8)/g;
                }
            if(y>=y_l && y<=y_h && r2g>r2g_l && r2g<r2g_h && b2g>b2g_l && b2g<b2g_h){
                sum_r = sum_r+r;
                sum_g = sum_g+g;
                sum_b = sum_b+b;
                //sum_r2g = sum_r2g + r2g;
                //sum_b2g = sum_b2g + b2g;
                countAll = countAll+1;
            }
            }//end x1
        }//end y1
        awbStats->u8GlobalR = (sum_r>>4)/countAll;
        awbStats->u8GlobalG = (sum_g>>4)/countAll;
        awbStats->u8GlobalB = (sum_b>>4)/countAll;
        awbStats->u8CountAll = (countAll)<<8/((img_width*img_height)>>2);

    ///////////////////////

    for(uint16 row_num = 0;row_num < AE_AWB_ZONE_ROW;row_num ++){
        for(uint16 col_num = 0;col_num < AE_AWB_ZONE_COLUMN;col_num ++){

            uint16 temp_start_y2 = awbGridInfo->au16GridYPos[row_num];
            uint16 temp_start_x2 = awbGridInfo->au16GridXPos[col_num];
            uint8 temp_BP = bayerPattLUT[bayer_pattern][temp_start_y2 & 0x1][temp_start_x2 & 0x1];
            r=0;
            g=0;
            b=0;
            sum_r=0;
            sum_g=0;
            sum_b=0;
            countAll =0;
            for(uint16 y2 = awbGridInfo->au16GridYPos[row_num]; (y2+1) < awbGridInfo->au16GridYPos[row_num + 1]; y2=y2+2){
                for(uint16 x2 = awbGridInfo->au16GridXPos[col_num]; (x2+1) < awbGridInfo->au16GridXPos[col_num + 1];x2=x2+2){


                    y = (img[y2*img_width+x2]+ img[y2*img_width+x2+1] + img[(y2+1)*img_width+x2] +img[(y2+1)*img_width+x2+1])>>6;
                    if(BPRG == temp_BP){
                        r = img[y2*img_width+x2];
                        g = (img[y2*img_width+x2+1] + img[(y2+1)*img_width+x2])>>1;
                        b = img[(y2+1)*img_width+x2+1];
                        r2g = (r<<8)/DIV_0_TO_1(g);
                        b2g = (b<<8)/DIV_0_TO_1(g);
                    }
                    else if(BPGR == temp_BP){
                        r = img[y2*img_width+x2+1];
                        g = (img[y2*img_width+x2] + img[(y2+1)*img_width+x2+1])>>1;
                        b = img[(y2+1)*img_width+x2];
                        r2g = (r<<8)/DIV_0_TO_1(g);
                        b2g = (b<<8)/DIV_0_TO_1(g);
                    }
                    else if(BPGB == temp_BP){
                        r = img[(y2+1)*img_width+x2];
                        g = (img[y2*img_width+x2] + img[(y2+1)*img_width+x2+1])>>1;
                        b = img[y2*img_width+x2];
                        r2g = (r<<8)/DIV_0_TO_1(g);
                        b2g = (b<<8)/DIV_0_TO_1(g);

                    }
                    else if (BPBG == temp_BP){
                        r = img[(y2+1)*img_width+x2+1];
                        g = (img[y2*img_width+x2+1] + img[(y2+1)*img_width+x2])>>1;
                        b = img[y2*img_width+x2];
                        r2g = (r<<8)/DIV_0_TO_1(g);
                        b2g = (b<<8)/DIV_0_TO_1(g);
                    }
                    if(y>y_l && y<y_h && r2g>r2g_l && r2g<r2g_h && b2g>b2g_l && b2g<b2g_h){
                        sum_r = sum_r+r;
                        sum_g = sum_g+g;
                        sum_b = sum_b+b;
                        //sum_r2g = sum_r2g + r2g;
                        //sum_b2g = sum_b2g + b2g;
                        countAll = countAll+1;
                    }
                }//au16GridXPos
            }//end au16GridYPos
            //printf("countAll:%ld\n",countAll);
            uint32 block_size = (awbGridInfo->au16GridYPos[1] - awbGridInfo->au16GridYPos[0])*(awbGridInfo->au16GridXPos[1]-awbGridInfo->au16GridXPos[0]);
            //uint16 ww = awbGridInfo->au16GridXPos[1]-awbGridInfo->au16GridXPos[0];
            //uint16 hh = awbGridInfo->au16GridYPos[1] - awbGridInfo->au16GridYPos[0];
            //printf("ww:%ld,hh:%ld\n",ww,hh);
            block_size = block_size >>2;

            if(0!=countAll){
                awbStats->u8ZoneAvgR[row_num*AE_AWB_ZONE_COLUMN+col_num]= (sum_r>>4)/DIV_0_TO_1(countAll);
                awbStats->u8ZoneAvgG[row_num*AE_AWB_ZONE_COLUMN+col_num]= (sum_g>>4)/DIV_0_TO_1(countAll);
                awbStats->u8ZoneAvgB[row_num*AE_AWB_ZONE_COLUMN+col_num]= (sum_b>>4)/DIV_0_TO_1(countAll);
                //printf("countAll:%ld\n",countAll);
                awbStats->u8ZoneCountAll[row_num*AE_AWB_ZONE_COLUMN+col_num]= (countAll<<8)/block_size;
                //printf("u8ZoneCountAll:%d,\n",awbStats->u8ZoneCountAll[row_num*AE_AWB_ZONE_COLUMN+col_num]);
            }
            else{
                awbStats->u8ZoneAvgR[row_num*AE_AWB_ZONE_COLUMN+col_num]= 0;
                awbStats->u8ZoneAvgG[row_num*AE_AWB_ZONE_COLUMN+col_num]= 0;
                awbStats->u8ZoneAvgB[row_num*AE_AWB_ZONE_COLUMN+col_num]= 0;
                awbStats->u8ZoneCountAll[row_num*AE_AWB_ZONE_COLUMN+col_num]= 0;
            }
       }//end col_num
    }//end row_num
    }//end else

}
void isp_ae_stats(uint16*img,img_info raw_info,striping_mode_cfg stripingModeCfg){

    uint16 img_height = raw_info.u16ImgHeight;
    uint16 img_width = raw_info.u16ImgWidth;
    uint16 start_x = 0;
    uint16 start_y = 0;

    uint16 striping_img_width;
    uint16 striping_img_start;


    ae_stats1 ae_stats1_info;
    ae_stats2 ae_stats2_info;
    ae_stats3 ae_stats3_info;
    ae_grid_info ae_grid;
    if(IS_STRIPING_MODE){
         //TO DO
#if 1
        /* ae_stats1_info */
        uint32 cnt = 0;
        ae_stats1_info.u32PixelCount = 0;
        memset(ae_stats1_info.u32HistogramMemArray, 0, 1024);
        for(uint8 sn1 = 0;sn1 < STRIPINGNUM ; sn1++){
            stripingModeCfg.u16StripingIndex = sn1;
            striping_img_start = get_striping_img_start(stripingModeCfg);
            striping_img_width = get_striping_img_width(stripingModeCfg);
             for (uint16 i = 0; i < 1024; i++) {
                for (uint16 y1 = 0; y1 < img_height; y1++) {
                    for (uint16 x1 = striping_img_start; x1 < striping_img_start + striping_img_width; x1++) {
                        if(0 == bayerPattLUT[0][y1 & 0x1][x1 & 0x1] && i == (img[y1 * img_width + x1] >>2)) { //only stats CHANNEL R in RAW
                            cnt++;
                        }

                    }
                }
                ae_stats1_info.u32PixelCount = ae_stats1_info.u32PixelCount + cnt;
                ae_stats1_info.u32HistogramMemArray[i] = ae_stats1_info.u32HistogramMemArray[i] + cnt;
                cnt = 0;
            }
        }//end stripping num++
#endif
        ///////////////////
#if 1
        /* ae_stats2_info */
        uint32 sumR =0,cntR=0;
        uint32 sumGR =0,cntGR=0;
        uint32 sumGB =0,cntGB=0;
        uint32 sumB =0,cntB=0;
        for(uint8 sn2 = 0;sn2 < STRIPINGNUM ; sn2++) {
            stripingModeCfg.u16StripingIndex = sn2;
            striping_img_start = get_striping_img_start(stripingModeCfg);
            striping_img_width = get_striping_img_width(stripingModeCfg);
            for (uint16 y2 = start_y; y2 < start_y + img_height; y2++) {
                for (uint16 x2 = striping_img_start; x2 < striping_img_start + striping_img_width; x2++) {
                    if (0 == bayerPattLUT[0][y2 & 0x1][x2 & 0x1]) {
                        cntR++;
                        sumR = sumR + img[y2 * img_width + x2];
                    }
                    if (1 == bayerPattLUT[0][y2 & 0x1][x2 & 0x1]) {
                        cntGR++;
                        sumGR = sumGR + img[y2 * img_width + x2];
                    }
                    if (2 == bayerPattLUT[0][y2 & 0x1][x2 & 0x1]) {
                        cntGB++;
                        sumGB = sumGB + img[y2 * img_width + x2];
                    }
                    if (3 == bayerPattLUT[0][y2 & 0x1][x2 & 0x1]) {
                        cntB++;
                        sumB = sumB + img[y2 * img_width + x2];
                    }
                }
            }
        }//end stripping num++
        ae_stats2_info.u16GlobalAvgR = sumR/DIV_0_TO_1(cntR);
        ae_stats2_info.u16GlobalAvgGR = sumGR/DIV_0_TO_1(cntGR);
        ae_stats2_info.u16GlobalAvgGB = sumGB/DIV_0_TO_1(cntGB);
        ae_stats2_info.u16GlobalAvgB = sumB/DIV_0_TO_1(cntB);
#endif
        ///////////////////
        /* ae_stats3_info */
        get_striping_gridX_info(ae_grid.au16GridXPos,AE_AWB_ZONE_COLUMN,stripingModeCfg);
        ae_grid.au16GridXPos[AE_AWB_ZONE_COLUMN] = img_width -1;
        calc_grid_info(0,img_height,AE_AWB_ZONE_ROW,ae_grid.au16GridYPos);
        ae_grid.au16GridYPos[AE_AWB_ZONE_ROW] = img_height-1;
        uint32 ctR = 0,ctGR = 0,ctGB = 0,ctB = 0;
        uint32 smR = 0,smGR = 0,smGB = 0,smB = 0;

        for(uint16 row_num = 0;row_num < AE_AWB_ZONE_ROW;row_num ++){
            for(uint16 col_num = 0;col_num < AE_AWB_ZONE_COLUMN;col_num ++){

                for(uint16 y3 = ae_grid.au16GridYPos[row_num]; y3 < ae_grid.au16GridYPos[row_num + 1]; y3++){
                    for(uint16 x3 = ae_grid.au16GridXPos[col_num]; x3 < ae_grid.au16GridXPos[col_num + 1];x3++){

                        if(0 == bayerPattLUT[0][y3 & 0x1][x3 & 0x1]){
                            ctR++;
                            smR = smR + img[y3*img_width+x3];
                            ae_stats3_info.u16ZoneAvg[row_num][col_num][0] = smR /DIV_0_TO_1(ctR);

                            //fprintf(data_head, "%d ",ae_stats3_info.u16ZoneAvg[row_num][col_num][0]);
                            ctR = 0;
                            smR = 0;

                        }
                        else if(1 == bayerPattLUT[0][y3 & 0x1][x3 & 0x1]){
                            ctGR++;
                            smGR = smGR + img[y3*img_width+x3];
                            ae_stats3_info.u16ZoneAvg[row_num][col_num][1] = smGR /DIV_0_TO_1(ctGR);
                            ctGR = 0;
                            smGR = 0;
                        }
                        else if(2 == bayerPattLUT[0][y3 & 0x1][x3 & 0x1]){
                            ctGB++;
                            smGB = smGB + img[y3*img_width+x3];
                            ae_stats3_info.u16ZoneAvg[row_num][col_num][2] = smGB /DIV_0_TO_1(ctGB);
                            ctGB = 0;
                            smGB = 0;
                        }
                        else if (3 == bayerPattLUT[0][y3 & 0x1][x3 & 0x1]){
                            ctB++;
                            smB = smB + img[y3*img_width+x3];
                            ae_stats3_info.u16ZoneAvg[row_num][col_num][3] = smB /DIV_0_TO_1(ctB);
                            ctB = 0;
                            smB = 0;

                        }

                    }
                }

            }
        }
        ///////////////////
    }

    else {
        //FE//
        /* ae_stats1_info */
        uint32 cnt = 0;
        ae_stats1_info.u32PixelCount = 0;
        for (uint16 i = 0; i < 1024; i++) {
            for (uint16 y1 = 0; y1 < img_height; y1++) {
                for (uint16 x1 = 0; x1 < img_width; x1++) {

                    if(0 == bayerPattLUT[0][y1 & 0x1][x1 & 0x1] && i == (uint16)(img[y1 * img_width + x1] >>2)){ //only stats CHANNEL R in RAW,shitf 12bit to 10bit
                        cnt++;
                    }
                }
            }
            ae_stats1_info.u32PixelCount = ae_stats1_info.u32PixelCount + cnt;
            ae_stats1_info.u32HistogramMemArray[i] = cnt;
            cnt = 0;
        }
        /////////////////////
        /* ae_stats2_info */
        uint32 sumR =0,cntR=0;
        uint32 sumGR =0,cntGR=0;
        uint32 sumGB =0,cntGB=0;
        uint32 sumB =0,cntB=0;
        for (uint16 y2 = 0; y2 < img_height; y2++) {
            for (uint16 x2 = 0; x2 < img_width; x2++) {

                if(0 == bayerPattLUT[0][y2 & 0x1][x2 & 0x1]){
                    cntR++;
                    sumR = sumR + img[y2 * img_width + x2];
                }
                if(1 == bayerPattLUT[0][y2 & 0x1][x2 & 0x1]){
                    cntGR++;
                    sumGR = sumGR + img[y2 * img_width + x2];
                }
                if(2 == bayerPattLUT[0][y2 & 0x1][x2 & 0x1]){
                    cntGB++;
                    sumGB = sumGB + img[y2 * img_width + x2];
                }
                if(3 == bayerPattLUT[0][y2 & 0x1][x2 & 0x1]){
                    cntB++;
                    sumB = sumB + img[y2 * img_width + x2];
                }
            }
        }
        ae_stats2_info.u16GlobalAvgR = sumR/DIV_0_TO_1(cntR);
        ae_stats2_info.u16GlobalAvgGR = sumGR/DIV_0_TO_1(cntGR);
        ae_stats2_info.u16GlobalAvgGB = sumGB/DIV_0_TO_1(cntGB);
        ae_stats2_info.u16GlobalAvgB = sumB/DIV_0_TO_1(cntB);
        //////////////////
        /*ae_stats3_info*/
        calc_grid_info(0,img_width,AE_AWB_ZONE_COLUMN,ae_grid.au16GridXPos);
        ae_grid.au16GridXPos[AE_AWB_ZONE_COLUMN] = img_width-1;
        calc_grid_info(0,img_height,AE_AWB_ZONE_ROW,ae_grid.au16GridYPos);
        ae_grid.au16GridYPos[AE_AWB_ZONE_ROW]  =  img_height-1;
        uint32 ctR = 0,ctGR = 0,ctGB = 0,ctB = 0;
        uint32 smR = 0,smGR = 0,smGB = 0,smB = 0;

        //FILE *data_head;
        //data_head = fopen("D:\\leetcode_project\\data_print_info.txt", "wb");

        for(uint16 row_num = 0;row_num < AE_AWB_ZONE_ROW;row_num ++){
            for(uint16 col_num = 0;col_num < AE_AWB_ZONE_COLUMN;col_num ++){

                for(uint16 y3 = ae_grid.au16GridYPos[row_num]; y3 < ae_grid.au16GridYPos[row_num + 1]; y3++){
                    for(uint16 x3 = ae_grid.au16GridXPos[col_num]; x3 < ae_grid.au16GridXPos[col_num + 1];x3++){

                        if(0 == bayerPattLUT[0][y3 & 0x1][x3 & 0x1]){
                            ctR++;
                            smR = smR + img[y3*img_width+x3];
                            ae_stats3_info.u16ZoneAvg[row_num][col_num][0] = smR /DIV_0_TO_1(ctR);

                            //fprintf(data_head, "%d ",ae_stats3_info.u16ZoneAvg[row_num][col_num][0]);
                            ctR = 0;
                            smR = 0;

                        }
                       else if(1 == bayerPattLUT[0][y3 & 0x1][x3 & 0x1]){
                            ctGR++;
                            smGR = smGR + img[y3*img_width+x3];
                            ae_stats3_info.u16ZoneAvg[row_num][col_num][1] = smGR /DIV_0_TO_1(ctGR);
                            ctGR = 0;
                            smGR = 0;
                        }
                        else if(2 == bayerPattLUT[0][y3 & 0x1][x3 & 0x1]){
                            ctGB++;
                            smGB = smGB + img[y3*img_width+x3];
                            ae_stats3_info.u16ZoneAvg[row_num][col_num][2] = smGB /DIV_0_TO_1(ctGB);
                            ctGB = 0;
                            smGB = 0;
                        }
                        else if (3 == bayerPattLUT[0][y3 & 0x1][x3 & 0x1]){
                            ctB++;
                            smB = smB + img[y3*img_width+x3];
                            ae_stats3_info.u16ZoneAvg[row_num][col_num][3] = smB /DIV_0_TO_1(ctB);
                            ctB = 0;
                            smB = 0;

                        }

                    }
                }

            }
        }
        //fclose(data_head);
        ///////////////////////////////
    } //end else
}

int main()
{
    /*   int in = 0;
       struct stat sb;
       in = open("lsc_test.raw", O_RDONLY);
       fstat(in, &sb);
       printf("File size is %ld\n", sb.st_size);
   */
    img_info raw_info;
    raw_info.u16StartX = 0;
    raw_info.u16StartY = 0;
    raw_info.u16ImgHeight = 1080;
    raw_info.u16ImgWidth = 1920;
    raw_info.u8bayermode = 0;
    striping_mode_cfg stripingModeCfg = {0,{0,640,1280},{640,640,640},0};
    uint16 blc_value[4] = {240,240,240,240};//RGGB
    //ae_stats ae_stats;
    const awb_stats_cfg awbStatsCfg = {5,245,64,320,64,320};
    //awbStatsCfg.y_l = 5;
    //awbStatsCfg.y_h = 245;
    //awbStatsCfg.r2g_l = 64;//base 256
    //awbStatsCfg.r2g_h = 320;//base 256
    //awbStatsCfg.b2g_l = 64;//base 256
    //awbStatsCfg.b2g_h = 320;//base 256

    awb_stats *awbStats;
    awbStats = (awb_stats *)malloc(sizeof(awb_stats));
    if(NULL == awbStats){
        printf("awbStats MALLOC FAIL!!!");
    }
    memset(awbStats,0,sizeof(awb_stats));

    awb_grid_info *awbGridInfo;
    awbGridInfo = (awb_grid_info *)malloc(sizeof(awb_grid_info));
    if(NULL == awbGridInfo){
        printf("awbGridInfo MALLOC FAIL!!!");
    }
    memset(awbGridInfo,0,sizeof(awb_grid_info));

    uint16* BayerImg =(uint16*) malloc(1920*1080*sizeof (uint16)) ;
    if(NULL == BayerImg){
        printf("BayerImg MALLOC FAIL!!!");
    }
    read_BayerImg("D:\\leetcode_project\\D65_CC_1920x1080_12bits_RGGB_Linear_20220316102320.raw",1080,1920,BayerImg);
    align_0_4095(BayerImg, blc_value,raw_info);//
    //isp_ae_stats(BayerImg,raw_info,stripingModeCfg);
    isp_awb_stats(BayerImg,raw_info,stripingModeCfg,awbStats,awbStatsCfg,awbGridInfo);
    //printf("read RAW ok!!!");

    free(BayerImg);
    free(awbGridInfo);
    free(awbStats);

    return 0;

}
