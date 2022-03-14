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
    striping_mode_cfg stripingModeCfg = {0,{0,640,1280},{640,640,640},0};
    uint16 blc_value[4] = {240,240,240,240};//RGGB
    //ae_stats ae_stats;
    //awb_stats awb_stats;
    uint16* BayerImg =(uint16*) malloc(1920*1080*sizeof (uint16)) ;
    read_BayerImg("D:\\leetcode_project\\CC_1920x1080_12bits_RGGB_Linear.raw",1080,1920,BayerImg);
    align_0_4095(BayerImg, blc_value,raw_info);//
    isp_ae_stats(BayerImg,raw_info,stripingModeCfg);
    printf("read RAW ok!!!");
    return 0;

}
