/*
 * Silicon labs si2177 Tuner Device Driver
 *
 * Author: Bobby Yang <bo.yang@amlogic.com>
 *
 *
 * Copyright (C) 2010 Amlogic Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


/* Standard Liniux Headers */
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <uapi/linux/dvb/frontend.h>
//#include <mach/gpio_data.h>
#include<mach/gpio.h>

/* Amlogic Headers */

/* Local Headers */
#include "si2177_func.h"
#ifdef CONFIG_AM_ATBM8869
#include "../atbm887x/atbm886x.h"
#endif

#define SI2177_TUNER_I2C_NAME           "si2177_tuner_i2c"
#define TUNER_DEVICE_NAME                "si2177"
#ifndef ERROR
#define ERROR                           1
#endif
#define CVD_SI2177_RSSI
static int si2177_debug = 1;
module_param(si2177_debug, int, 0644);
MODULE_PARM_DESC(si2177_debug, "\n si2177_debug \n");

static unsigned short audio_delay = 100;
module_param(audio_delay, ushort, 0644);
MODULE_PARM_DESC(audio_delay, "\n the delay for audio mode detection by ms\n");

#define siprintk if(si2177_debug) printk
#define ssiprintk if(si2177_debug==2) printk

struct si2177_device_s *si2177_devp;
static void get_mono_sound_mode(unsigned int *);
void si2177_set_frequency(unsigned int freq);
static void si2177_set_std(void);
int si2177_get_strength(void);
static int si2177_xout_on(void);


int si2177_get_strength(void){
	  int strength=100;
	  if(si2177_tuner_status(&si2177_devp->tuner_client, SI2177_ATV_STATUS_CMD_INTACK_OK, &si2177_devp->si_cmd_reply)!=0)
    	 {
        	printk("[si2177..]%s:get si2177 tuner status error!!!\n",__func__);
        	return -ERROR;
         }
    	else
        	strength = si2177_devp->si_cmd_reply.tuner_status.rssi;//-256;
		return strength;
}


static void sound_store(const char *buff, v4l2_std_id *std)
{
        if(!strncmp(buff,"dk",2))
                *std |= V4L2_STD_PAL_DK;
        else if(!strncmp(buff,"bg",2))
                *std |= V4L2_STD_B;
        else if(!strncmp(buff,"i",1))
                *std |= V4L2_STD_PAL_I;
        else if(!strncmp(buff,"nm",2))
                *std |= V4L2_STD_NTSC;
        else if(!strncmp(buff,"pm",2))
                *std |= V4L2_STD_PAL_M;
        else if(!strncmp(buff,"l",1))
                *std |= V4L2_STD_SECAM_L;
        else if(!strncmp(buff,"lc",2))
                *std |= V4L2_STD_SECAM_LC;
		else if(!strncmp(buff,"n",1))
                *std |= V4L2_STD_PAL_N;
        else
                pr_info("invaild command.\n");
}
static ssize_t si2177_store(struct class *cls, struct class_attribute *attr, const char *buf, size_t count)
{
 /*       int n = 0;
        unsigned char ret =0;
        char *buf_orig, *ps, *token;
        char *parm[4];
        unsigned int addr = 0, val = 0;
        v4l2_std_id  std;
        buf_orig = kstrdup(buf, GFP_KERNEL);
        ps = buf_orig;
        while (1) {
                token = strsep(&ps, " \n");
                if (token == NULL)
                        break;
                if (*token == '\0')
                        continue;
                parm[n++] = token;
        }

        if (!strncmp(parm[0],"rs",strlen("rs")))
        {
                if (n != 2)
                {
                        pr_err("read: invalid parameter\n");
                        kfree(buf_orig);
                        return count;
                }
                addr = simple_strtol(parm[1], NULL, 16);
                pr_err("%s 0x%x\n", parm[0], addr);
                si2177_get_property(&si2177_devp->tuner_client, 0, addr, &si2177_devp->si_cmd_reply);
                pr_info("%s: 0x%x --> 0x%x\n", parm[0], addr, si2177_devp->si_cmd_reply.get_property.data);
        }
        else if (!strncmp(parm[0],"ws",strlen("ws")))
        {
                addr = simple_strtol(parm[1], NULL, 16);
                val  = simple_strtol(parm[2], NULL, 16);
                pr_err("%s 0x%x 0x%x", parm[0], addr, val);
                si2177_set_property(&si2177_devp->tuner_client,0,addr,val,&si2177_devp->si_cmd_reply);
                pr_info("%s: 0x%x <-- 0x%x\n", parm[0], addr, val);
        }
        else if(!strncmp(parm[0],"atv_restart",strlen("atv_restart")))
        {
                ret = si2177_atv_restart(&si2177_devp->tuner_client, 0,  &si2177_devp->si_cmd_reply);
                if(ret)
                        pr_info("[tuner..] atv_restart error.\n");
        }
        else if(!strncmp(parm[0],"dtv_restart",strlen("atv_restart")))
        {
                ret = si2177_dtv_restart(&si2177_devp->tuner_client, &si2177_devp->si_cmd_reply);
                if(ret)
                        pr_info("[tuner..] atv_restart error.\n");
        }
        else if(!strcmp(parm[0],"tune"))
        {
                val  = simple_strtol(parm[1], NULL, 10);
                //SI2177_TUNER_TUNE_FREQ_CMD_MODE_ATV = 1
                si2177_tune(&si2177_devp->tuner_client, si2177_devp->parm.mode, val, &si2177_devp->si_cmd_reply, &si2177_devp->si_common_reply);
        }
        else if(!strcmp(parm[0],"tuner_status"))
        {
                //1=SI2177_ATV_STATUS_CMD_INTACK_CLEAR
                si2177_tuner_status(&si2177_devp->tuner_client, 1, &si2177_devp->si_cmd_reply);
                printk("\n[tuner_status.vco_code]:%d        \n[tuner_status.tc]:%d     \n[tuner_status.rssil]:%d  \n[tuner_status.rssih]:%d"
                                "\n[tuner_status.rssi]:%d   \n[tuner_status.freq]:%d \n[tuner_status.mode]:%d \n", si2177_devp->si_cmd_reply.tuner_status.vco_code,
                                si2177_devp->si_cmd_reply.tuner_status.tc, si2177_devp->si_cmd_reply.tuner_status.rssil, si2177_devp->si_cmd_reply.tuner_status.rssih,
                                si2177_devp->si_cmd_reply.tuner_status.rssi,  si2177_devp->si_cmd_reply.tuner_status.freq,   si2177_devp->si_cmd_reply.tuner_status.mode);
        }
        else if(!strncmp(parm[0],"atv_status",strlen("atv_status")))
        {
                //1=SI2177_ATV_STATUS_CMD_INTACK_CLEAR
                si2177_cmdreplyobj_t *si_cmd_reply = &si2177_devp->si_cmd_reply;
                si2177_atv_status(&si2177_devp->tuner_client, 1, si_cmd_reply);
                printk("\n[atv_status.chl]:%d   \n[atv_status.pcl]:%d       \n[atv_status.dl]:%d   \n[atv_status.snrl]:%d"
                                "\n[atv_status.snrh]:%d  \n[atv_status.video_snr]:%d    \n[atv_status.afc_freq]:%d    \n[atv_status.video_sc_spacing]:%d"
                                "\n[atv_status.video_sys]:%d    \n[atv_status.color]:%d      \n[atv_status.audio_chan_bw]:%d \n[atv_status.audio_sys]:%d"
                                "\n[atv_status.audio_demod_mode]:%d     \n[atv_status.sound_level]:%d       \n[atv_status.trans/lines]:%d.\n",
                                si_cmd_reply->atv_status.chl,   si_cmd_reply->atv_status.pcl,  si_cmd_reply->atv_status.dl,   si_cmd_reply->atv_status.snrl,
                                si_cmd_reply->atv_status.snrh,  si_cmd_reply->atv_status.video_snr,   si_cmd_reply->atv_status.afc_freq,  si_cmd_reply->atv_status.video_sc_spacing,
                                si_cmd_reply->atv_status.video_sys,    si_cmd_reply->atv_status.color, si_cmd_reply->atv_status.audio_chan_bw,  si_cmd_reply->atv_status.audio_sys,
                                si_cmd_reply->atv_status.audio_demod_mode,  si_cmd_reply->atv_status.sound_level,   si_cmd_reply->atv_status.lines);
        }
        else if(!strncmp(parm[0],"mono_mode",strlen("mono_mode")))
        {
                get_mono_sound_mode(&val);
                printk("[si2177..]%s get mono 0x%x,mode %s.\n",__func__,val,v4l2_std_to_str((v4l2_std_id)val));
        }
        else if(!strncmp(parm[0],"enter_atv",strlen("enter_atv")))
        {
                si2177_devp->parm.mode                 = SI2177_TUNER_TUNE_FREQ_CMD_MODE_ATV;
                si2177_init(&si2177_devp->tuner_client,&si2177_devp->si_cmd_reply,&si2177_devp->si_common_reply);
                si2177_configure(&si2177_devp->tuner_client,&si2177_devp->si_prop,&si2177_devp->si_cmd_reply,&si2177_devp->si_common_reply);
        }
        else if(!strncmp(parm[0],"std",strlen("std")))
        {
                if(!strncmp(parm[1],"pal",3))
                {
                        std= V4L2_COLOR_STD_PAL;
                        sound_store(parm[2],&std);
                }
                else if(!strncmp(parm[1],"ntsc",4))
                {
                        std= V4L2_COLOR_STD_NTSC;
                        sound_store(parm[2],&std);
                }
                else if(!strncmp(parm[1],"secam",5))
                {
                        std= V4L2_COLOR_STD_SECAM;
                        sound_store(parm[2],&std);
                }
                si2177_devp->parm.std  =std;
                si2177_set_std();
                siprintk("[si2177..]%s set std color %s, audio type %s.\n",__func__,\
                                v4l2_std_to_str(0xff000000&si2177_devp->parm.std), v4l2_std_to_str(0xffffff&si2177_devp->parm.std));
        }
		else if(!strncmp(parm[0],"cvbs_amp",strlen("cvbs_amp")))
		{
			si2177_devp->si_prop.atv_cvbs_out.amp=simple_strtol(parm[1], NULL, 10);
			if(si2177_set_property(&si2177_devp->tuner_client, 0,0x609,((si2177_devp->si_prop.atv_cvbs_out.amp & 0xff)<<8)|(25 & 0xff), &si2177_devp->si_cmd_reply))
            printk("[si2177..]%s set cvbs_amp out error.\n",__func__);

		}
		else if(!strncmp(parm[0],"xout",strlen("xout")))
		{
			if(!si2177_xout_on())
				printk("xoout is ok\n");

		}
		else
                printk("invalid command\n");
        kfree(buf_orig);
        return count;*/
}

static ssize_t si2177_show(struct class *cls,struct class_attribute *attr,char *buff)
{
        return 0;
}
static CLASS_ATTR(si2177_debug,0644,si2177_show,si2177_store);

#ifdef  CVD_SI2177_RSSI
int cvd_get_rf_strength(void)
{
	 int cvd_rssi=0;
	 if(!si2177_devp)
	 	return 0;
	 cvd_rssi=si2177_devp->si_cmd_reply.tuner_status.rssi;
	 return ((cvd_rssi>0)?cvd_rssi:0);
}
EXPORT_SYMBOL(cvd_get_rf_strength);
#endif


static int si2177_get_rf_strength(struct dvb_frontend *fe, unsigned short *strength)
{
        if(!strength)
        {
                printk("[si2177..]%s null pointer error.\n",__func__);
                return -ERROR;
        }
        if(si2177_tuner_status(&si2177_devp->tuner_client, SI2177_ATV_STATUS_CMD_INTACK_OK, &si2177_devp->si_cmd_reply)!=0)
        {
                pr_info("[si2177..]%s:get si2177 tuner status error!!!\n",__func__);
                return -ERROR;
        }
        else
                *strength = si2177_devp->si_cmd_reply.tuner_status.rssi;
        return 0;
}

static int si2177_get_tuner_status(struct dvb_frontend *dvb_fe, tuner_status_t *status)
{
        si2177_cmdreplyobj_t  *si_cmd_reply = &si2177_devp->si_cmd_reply;
        if(!status)
        {
                printk("[si2177..]%s null pointer error.\n",__func__);
                return -ERROR;
        }
        //SI2177_ATV_STATUS_CMD_INTACK_OK = 0
        if(si2177_tuner_status(&si2177_devp->tuner_client, 0, si_cmd_reply)!=0)
        {
                printk("[si2177..]%s:get si2177 tuner status error.\n",__func__);
                return -ERROR;
        }
        status->frequency = si_cmd_reply->tuner_status.freq;
        status->mode        = si_cmd_reply->tuner_status.mode;
        status->rssi           = si_cmd_reply->tuner_status.rssi;
        if(si_cmd_reply->tuner_status.rssih ||si_cmd_reply->tuner_status.rssil)
        {
                status->tuner_locked = 0;
                siprintk("[si2177..]%s rssi %d dismatch <%d, %d>.\n",__func__, (si_cmd_reply->tuner_status.rssi<<24),
                                (si2177_devp->si_prop.atv_rsq_rssi_threshold.lo),(si2177_devp->si_prop.atv_rsq_rssi_threshold.hi));
        }
        else
        {
                status->tuner_locked = 1;
                siprintk("[si2177..]%s rssi %d dismatch <%d, %d>.\n",__func__, si_cmd_reply->tuner_status.rssi,
                                si2177_devp->si_prop.atv_rsq_rssi_threshold.lo,si2177_devp->si_prop.atv_rsq_rssi_threshold.hi);
        }
        siprintk(" frequency %u, mode %d,tuner locked %d.\n",status->frequency, status->mode, status->tuner_locked);
        return 0;
}

static int  si2177_tuner_fine_tune(struct dvb_frontend *dvb_fe, int offset_khz)
{

        if(si2177_fine_tune(&si2177_devp->tuner_client,0,0,offset_khz<<1,&si2177_devp->si_cmd_reply)!=0)
        {
                printk("[si2177..]%s: si2177 fine tune error.\n",__func__);
                return -ERROR;
        }
        else
        {
                if(si2177_debug)
                        printk("[si2177..]%s: si2177 fine tune %d khz.\n", __func__, offset_khz);
                return 0;
        }
}
static int si2177_xout_on()
{
    int return_code;
    printk("Si2177_XoutOn:  Turning Xout ON\n");

    if ((return_code = si2177_config_clocks(&si2177_devp->tuner_client,
                                              SI2177_CONFIG_CLOCKS_CMD_SUBCODE_CODE,
                                              SI2177_POWER_UP_CMD_CLOCK_MODE_XTAL,
                                              SI2177_CONFIG_CLOCKS_CMD_EN_XOUT_EN_XOUT,
                                              &si2177_devp->si_cmd_reply)) != NO_SI2177_ERROR)
    	return return_code;

    return NO_SI2177_ERROR;
}


void si2177_set_frequency(unsigned int freq)
{
        int ret = 0;
        if(si2177_debug)
	    printk("[%s]:now mode is %d(0-DTV,1-ATV)\n",__func__,si2177_devp->parm.mode);
        ret = si2177_tune(&si2177_devp->tuner_client, si2177_devp->parm.mode, freq, &si2177_devp->si_cmd_reply, &si2177_devp->si_common_reply);
        if (ret)
                siprintk("[si2177..]%s: tune frequency error:%d.\n", __func__, ret);
}
static void si2177_set_std(void)
{
        unsigned char color_mode=SI2177_ATV_VIDEO_MODE_PROP_COLOR_PAL_NTSC;
	unsigned char video_sys=SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_I;
	unsigned char  ret=0;
        v4l2_std_id ptstd = si2177_devp->parm.std;
        /*set color standard of atv:pal & ntsc*/
    /*    if (ptstd & (V4L2_COLOR_STD_PAL | V4L2_COLOR_STD_NTSC))
        {
                color_mode = SI2177_ATV_VIDEO_MODE_PROP_COLOR_PAL_NTSC;
        }
        else if (ptstd & (V4L2_COLOR_STD_SECAM))
        {
                color_mode = SI2177_ATV_VIDEO_MODE_PROP_COLOR_SECAM;
        }*/

        /* set audio standard of tuner:secam*/
        if (ptstd & V4L2_STD_B)
        {
                si2177_devp->fre_offset = 2250000;
                video_sys = SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_B;
        }
        else if (ptstd & V4L2_STD_GH)
        {
                si2177_devp->fre_offset = 2250000;
                video_sys = SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_GH;
        }
        else if ((ptstd & V4L2_STD_NTSC) || ((ptstd & V4L2_STD_PAL_M)))
        {
                si2177_devp->fre_offset = 1750000;
                video_sys = SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_M;
        }
        else if (ptstd & (V4L2_STD_PAL_N | V4L2_STD_PAL_Nc))
        {
                si2177_devp->fre_offset = 1750000;
                video_sys = SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_N;
        }
        else if (ptstd & V4L2_STD_PAL_I)
        {
                si2177_devp->fre_offset = 2750000;
                video_sys = SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_I;
        }
        else if (ptstd & V4L2_STD_DK)
        {
                si2177_devp->fre_offset = 2750000;
                video_sys = SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_DK;
        }
        else if (ptstd & V4L2_STD_SECAM_L)
        {
                si2177_devp->fre_offset = 2750000;
                video_sys = SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_L;
        }
        else if (ptstd & V4L2_STD_SECAM_LC)
        {
                si2177_devp->fre_offset = 2750000;
                video_sys = SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_LP;
        }
        ret = si2177_set_property(&si2177_devp->tuner_client, 0, 0x604, color_mode<<4|video_sys, &si2177_devp->si_cmd_reply);
        if(ret)
                printk("[si2177..]%s:set std %llu error\n",__func__, (unsigned long long)ptstd);
        siprintk("[si2177..]%s: color %d,video_mode.video_sys:%d.\n", __func__, color_mode, video_sys);
        if(si2177_atv_restart(&si2177_devp->tuner_client, 0,  &si2177_devp->si_cmd_reply))
                printk("[si2177..]%s: atv restart error.\n", __func__);
}

static int si2177_set_params(struct dvb_frontend *fe)
{
	struct dtv_frontend_properties *c = &fe->dtv_property_cache;
    int strength=100;
    if(FE_ANALOG == fe->ops.info.type)
    {
	    if(c->analog.std != si2177_devp->parm.std)
	    {
	        si2177_devp->parm.std  = c->analog.std;
	        si2177_set_std();
                siprintk("[si2177..]%s set std color %s, audio type %s.\n",__func__,\
                            v4l2_std_to_str(0xff000000&si2177_devp->parm.std), v4l2_std_to_str(0xffffff&si2177_devp->parm.std));
	    }
            if(c->frequency  != si2177_devp->parm.frequency)
	    {
	        si2177_devp->parm.frequency  = c->frequency;
	        si2177_set_frequency(si2177_devp->parm.frequency + si2177_devp->fre_offset);
                siprintk("[si2177..]%s set frequency %u. frequency offset is %d.\n",__func__,si2177_devp->parm.frequency,si2177_devp->fre_offset);
	    }
        /*
	    else if(parm->u.analog.audmode != si2177_devp->parm.audmode)
	    {
	        printk("[si2177..] %s no audmode case catch error.\n",__func__);
	    }
	    else if(parm->u.analog.soundsys != si2177_devp->parm.soundsys)
	    {
		printk("[si2177..] %s no soundsys case catch error.\n",__func__);
	    }
            else if(parm->u.analog.reserved != si2177_devp->parm.reserved)
            {
                printk("[si2177..] %s no case catch resevred.\n",__func__);
            }*/
	}
    else
	{
	/*	if(FE_QAM == fe->ops.info.type)
		{
			if(QAM_128 == parm->u.qam.modulation)
			{
				si2177_devp->si_prop.dtv_lif_out.amp = 22;
				if(si2177_set_property(&si2177_devp->tuner_client,0,0x707,22,&si2177_devp->si_cmd_reply))
					printk("[si2177..]%s set dtv lif out amp 22error.\n",__func__);
			}
			else
			{
				si2177_devp->si_prop.dtv_lif_out.amp = 25;
				if(si2177_set_property(&si2177_devp->tuner_client,0,0x707,25,&si2177_devp->si_cmd_reply))
					printk("[si2177..]%s set dtv lif out amp 25 error.\n",__func__);
			}
		}*/
		 if(FE_ATSC==fe->ops.info.type){
				si2177_devp->si_prop.dtv_lif_freq.offset=6350;
				printk("FE_ATSC set para if is %d\n",si2177_devp->si_prop.dtv_lif_freq.offset);
				if(si2177_set_property(&si2177_devp->tuner_client,0,0x706,si2177_devp->si_prop.dtv_lif_freq.offset,&si2177_devp->si_cmd_reply))
					printk("[si2177..]%s set dtv lif out if error.\n",__func__);
			//	parm->frequency=parm->frequency+2500000;//for atsc 2.5mhz
		}else if(FE_QAM == fe->ops.info.type){
			si2177_devp->si_prop.dtv_lif_freq.offset=6000;
			printk("FE_QAM set para if is %d\n",si2177_devp->si_prop.dtv_lif_freq.offset);
			if(si2177_set_property(&si2177_devp->tuner_client,0,0x706,si2177_devp->si_prop.dtv_lif_freq.offset,&si2177_devp->si_cmd_reply))
					printk("[si2177..]%s set dtv lif out if error.\n",__func__);
			if(c->modulation==QAM_256){
				si2177_devp->si_prop.dtv_lif_out.amp = 23;
			}else{
				si2177_devp->si_prop.dtv_lif_out.amp = 23;
			}
			printk("amp is %d,offset is %d,amp+offset is %x\n",si2177_devp->si_prop.dtv_lif_out.amp,si2177_devp->si_prop.dtv_lif_out.offset,((si2177_devp->si_prop.dtv_lif_out.offset)+((si2177_devp->si_prop.dtv_lif_out.amp)<<8)));
			if(si2177_set_property(&si2177_devp->tuner_client,0,0x707,(si2177_devp->si_prop.dtv_lif_out.offset)+((si2177_devp->si_prop.dtv_lif_out.amp)<<8),&si2177_devp->si_cmd_reply))
					printk("[si2177..]%s set dtv lif out amp error.\n",__func__);
		}else if(FE_DTMB == fe->ops.info.type){
			si2177_devp->si_prop.dtv_lif_out.amp = 21;
			if(si2177_set_property(&si2177_devp->tuner_client,0,0x707,(si2177_devp->si_prop.dtv_lif_out.offset)+((si2177_devp->si_prop.dtv_lif_out.amp)<<8),&si2177_devp->si_cmd_reply))
					printk("[si2177..]%s set dtv lif out amp error.\n",__func__);

			printk("FE_DTMB amp is %d,offset is %d,amp+offset is %x\n",si2177_devp->si_prop.dtv_lif_out.amp,si2177_devp->si_prop.dtv_lif_out.offset,((si2177_devp->si_prop.dtv_lif_out.offset)+((si2177_devp->si_prop.dtv_lif_out.amp)<<8)));

		}

		si2177_devp->parm.frequency  = c->frequency;
		si2177_set_frequency(si2177_devp->parm.frequency);
		printk("[si2177..]%s set frequency %u. \n",__func__,si2177_devp->parm.frequency);
		if(si2177_tuner_status(&si2177_devp->tuner_client, SI2177_ATV_STATUS_CMD_INTACK_OK, &si2177_devp->si_cmd_reply)!=0)
		{
        	printk("[si2177..]%s:get si2177 tuner status error!!!\n",__func__);
        	return -ERROR;
		}
		else
		    strength = si2177_devp->si_cmd_reply.tuner_status.rssi-256;
		printk("strength is %d\n",strength);
     }

    return 0;
}
//try audmode B,CH,I,DK,return the sound level
static unsigned char set_video_audio_mode(unsigned char color,unsigned char audmode)
{
        int ret = 0;
        ret = si2177_set_property(&si2177_devp->tuner_client, 0, 0x604, color<<4|audmode, &si2177_devp->si_cmd_reply);
        ret = si2177_atv_restart(&si2177_devp->tuner_client, 0,  &si2177_devp->si_cmd_reply);
        mdelay(audio_delay);
        if(ret)
                printk("[si2177..]%s set sound mode %d error.\n",__func__,audmode);
        ret = si2177_atv_status(&si2177_devp->tuner_client, SI2177_ATV_STATUS_CMD_INTACK_OK, &si2177_devp->si_cmd_reply);
        if(ret)
                printk("[si2177..]%s:get si2177 atv status error.\n",__func__);
        if(!ret)
                return si2177_devp->si_cmd_reply.atv_status.sound_level;
        else
                return ret;
}
static void get_mono_sound_mode(unsigned int *mode)
{
        unsigned char max_level=0, cu_level=0;
        //set audio mode B=0,GH=1,I=4,DK=5
        cu_level = set_video_audio_mode(si2177_devp->si_prop.atv_video_mode.color,SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_B);
        if(cu_level > max_level)
        {
                max_level = cu_level;
                *mode = V4L2_STD_B;
        }
        cu_level = set_video_audio_mode(si2177_devp->si_prop.atv_video_mode.color,SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_GH);
        if(cu_level > max_level)
        {
                max_level = cu_level;
                *mode = V4L2_STD_GH;
        }
        cu_level = set_video_audio_mode(si2177_devp->si_prop.atv_video_mode.color,SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_I);
        if(cu_level > max_level)
        {
                max_level = cu_level;
                *mode = V4L2_STD_PAL_I;
        }
        cu_level = set_video_audio_mode(si2177_devp->si_prop.atv_video_mode.color,SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_DK);
        if(cu_level > max_level)
        {
                max_level = cu_level;
                *mode = V4L2_STD_DK;
        }
        siprintk("[si2177..]%s auto result audio mode is 0x%x,%s.\n",__func__, *mode,v4l2_std_to_str((v4l2_std_id)*mode));
        //revert the color&audmode
        si2177_set_property(&si2177_devp->tuner_client, 0, 0x604, si2177_devp->si_prop.atv_video_mode.color<<4|si2177_devp->si_prop.atv_video_mode.video_sys, &si2177_devp->si_cmd_reply);
        si2177_atv_restart(&si2177_devp->tuner_client, 0,  &si2177_devp->si_cmd_reply);
}
static int si2177_set_config(struct dvb_frontend *fe, void *arg)
{
        tuner_param_t *param = (tuner_param_t*)arg;
        if(!arg)
        {
                printk("[si2177..]%s null pointer error.\n",__func__);
                return -ERROR;
        }
        switch(param->cmd)
        {
                case TUNER_CMD_AUDIO_MUTE:
                        //SI2177_ATV_AF_OUT_PROP = 0x060b
                        if(si2177_set_property(&si2177_devp->tuner_client, 0,0x60b, 0, &si2177_devp->si_cmd_reply))
                        {
                                printk("[si2177..]%s mute tuner error.\n",__func__);
                                return -ERROR;
                        }
                        break;
                case TUNER_CMD_AUDIO_ON:
                        if(si2177_set_property(&si2177_devp->tuner_client, 0,0x60b, si2177_devp->si_prop.atv_af_out.volume, &si2177_devp->si_cmd_reply))
                        {
                                printk("[si2177..]%s set tuner volume error.\n",__func__);
                                return -ERROR;
                        }
                        break;
                case TUNER_CMD_TUNER_POWER_DOWN:
                        if(si2177_power_down(&si2177_devp->tuner_client, &si2177_devp->si_cmd_reply))
                        {
                                printk("[si2177..]%s power down tuner error.\n",__func__);
                                return -ERROR;
                        }
                        siprintk("[si2177..]%s power down tuner TUNER_CMD_TUNER_POWER_DOWN.\n",__func__);
                        break;
                case TUNER_CMD_TUNER_POWER_ON:
                        if(si2177_init(&si2177_devp->tuner_client, &si2177_devp->si_cmd_reply, &si2177_devp->si_common_reply))
                        {
                                pr_info("[si2177..]%s: si2177 initializate error.\n",__func__);
                                return -ERROR;
                        }
                        if(si2177_configure(&si2177_devp->tuner_client, &si2177_devp->si_prop, &si2177_devp->si_cmd_reply, &si2177_devp->si_common_reply))
                                printk("[si2177..]%s: si2177 configure atv&dtv error.\n",__func__);
                        break;
                case TUNER_CMD_SET_VOLUME:
                        if(param->parm > SI2177_ATV_AF_OUT_PROP_VOLUME_VOLUME_MAX)
                                param->parm = SI2177_ATV_AF_OUT_PROP_VOLUME_VOLUME_MAX;
                        si2177_devp->si_prop.atv_af_out.volume = param->parm;
                        if(si2177_set_property(&si2177_devp->tuner_client, 0,0x60b, si2177_devp->si_prop.atv_af_out.volume, &si2177_devp->si_cmd_reply))
                                printk("[si2177..]%s set tuner volume error.\n",__func__);
                        break;
                case TUNER_CMD_GET_MONO_MODE:
                        get_mono_sound_mode(&param->parm);
                        break;
                case TUNER_CMD_SET_LEAP_SETP_SIZE:
                        si2177_devp->parm.leap_step = param->parm;
                        break;
                case TUNER_CMD_SET_BEST_LOCK_RANGE:
                        break;
                case TUNER_CMD_GET_BEST_LOCK_RANGE:
                        break;
		case TUNER_CMD_SET_CVBS_AMP_OUT:
			si2177_devp->si_prop.atv_cvbs_out.amp=param->parm;
			if(si2177_set_property(&si2177_devp->tuner_client, 0,0x609,((si2177_devp->si_prop.atv_cvbs_out.amp & 0xff)<<8)|(25 & 0xff), &si2177_devp->si_cmd_reply))
			printk("[si2177..]%s set cvbs_amp out error.\n",__func__);
			break;
		case TUNER_CMD_GET_CVBS_AMP_OUT:
			if(si2177_get_property(&si2177_devp->tuner_client,0,0x609,&si2177_devp->si_cmd_reply))
			{
			printk("[si2177..]:%s:get cvbs amp error.\n",__func__);
			}
			param->parm=si2177_devp->si_cmd_reply.get_property.data;
			printk("[si2177..]%s:get the cvbs amp out is %d\n",__func__,param->parm);
			break;
                default:
                        break;
        }
        return 0;
}
static void si2177_get_status(struct dvb_frontend *fe, void *stat);
static void si2177_get_pll_status(struct dvb_frontend *fe, void *stat);

static int si2177_tuner_get_ops(struct aml_fe_dev *dev, int mode, void *ops)
{
        struct dvb_tuner_ops *si2177_tuner_ops  = (struct dvb_tuner_ops*)ops;
        if(!ops)
        {
                printk("[si2177..]%s null pointer error.\n",__func__);
                return -ERROR;
        }
        si2177_tuner_ops->info.frequency_min    = SI2177_TUNER_TUNE_FREQ_CMD_FREQ_MIN;
        si2177_tuner_ops->info.frequency_max   = SI2177_TUNER_TUNE_FREQ_CMD_FREQ_MAX;
        switch(mode)
        {
                case AM_FE_QPSK:
                        //SI2177_DTV_MODE_PROP_MODULATION_ISDBT = 4
                        si2177_devp->si_prop.dtv_mode.modulation = 4;
                        break;
                case AM_FE_QAM:
                        //SI2177_DTV_MODE_PROP_MODULATION_DVBC = 3
                        si2177_devp->si_prop.dtv_mode.modulation = 3;
                        break;
                case AM_FE_OFDM:
                        //SI2177_DTV_MODE_PROP_MODULATION_DVBT = 2
                        si2177_devp->si_prop.dtv_mode.modulation = 2;
                        break;
                case AM_FE_ATSC:
                        //SI2177_DTV_MODE_PROP_MODULATION_ATSC = 0
                        si2177_devp->si_prop.dtv_mode.modulation = 0;
                        break;
                case AM_FE_ANALOG:
                        break;
				case AM_FE_DTMB:
                        //SI2176_DTV_MODE_PROP_MODULATION_DTMB = 6
                        si2177_devp->si_prop.dtv_mode.modulation = 6;
                        break;
                default:
                        printk("[si2177..] %s no mode to match %d.\n",__func__, mode);
                        break;
        }

        if(AM_FE_ANALOG == mode)//the auto afc step size 1/2 M
        {
                si2177_tuner_ops->info.frequency_step = 0x0;
                //fine tune
                si2177_tuner_ops->fine_tune             = si2177_tuner_fine_tune;
                //the box for expand
                si2177_tuner_ops->set_config           = si2177_set_config;
                si2177_devp->parm.mode                 = SI2177_TUNER_TUNE_FREQ_CMD_MODE_ATV;
        }
        else//dtv doesn't use auto afc
        {
                if(si2177_set_property(&si2177_devp->tuner_client, 0, 0x703,
                                        si2177_devp->si_prop.dtv_mode.modulation<<4|8,
                                        &si2177_devp->si_cmd_reply))
                        printk("[si2177..] %s mode %d si2177_set_property error.",__func__, mode);
                si2177_devp->parm.mode                 = SI2177_TUNER_TUNE_FREQ_CMD_MODE_DTV;
                si2177_tuner_ops->info.frequency_step = 0x0;
        }
        si2177_tuner_ops->get_rf_strength       = si2177_get_rf_strength;
        si2177_tuner_ops->get_tuner_status    = si2177_get_tuner_status;
        si2177_tuner_ops->get_status              =si2177_get_status;
	si2177_tuner_ops->get_pll_status              =si2177_get_pll_status;
        //set std&frequency
        si2177_tuner_ops->set_params     	  = si2177_set_params;

        return 0;
}

#define TUNER_POWER_GPIO_OWNER  "TUNER_OWNER"

static int si2177_tuner_fe_init(struct aml_fe_dev *dev)
{

        int error_code = 0;
        if(!dev)
        {
                printk("[si2177..]%s: null pointer error.\n",__func__);
                return -ERROR;
        }
        si2177_devp->tuner_client.adapter = dev->i2c_adap;
        si2177_devp->tuner_client.addr    = dev->i2c_addr;
        if(!sprintf(si2177_devp->tuner_client.name,SI2177_TUNER_I2C_NAME))
        {
                printk("[si2177..]%s sprintf name error.\n",__func__);
        }
		amlogic_gpio_request(dev->reset_gpio,TUNER_POWER_GPIO_OWNER);//lock

	   amlogic_gpio_direction_output(dev->reset_gpio,0,TUNER_POWER_GPIO_OWNER);
	   udelay(400);
	   amlogic_gpio_direction_output(dev->reset_gpio,1,TUNER_POWER_GPIO_OWNER);

		amlogic_gpio_free(dev->reset_gpio,TUNER_POWER_GPIO_OWNER); //free

  /*     gpio_out(dev->reset_gpio,0);
       udelay(400);
       gpio_out(dev->reset_gpio,1);*/
       printk("[%s]tuner reset.dev->reset_value=[%d],i2c_addr is %x \n",__func__,dev->reset_value,dev->i2c_addr);
      return error_code;
}

static int si2177_enter_mode(struct aml_fe *fe, int mode)
{
        int err_code;
        err_code = si2177_init(&si2177_devp->tuner_client,&si2177_devp->si_cmd_reply,&si2177_devp->si_common_reply);
        err_code = si2177_configure(&si2177_devp->tuner_client,&si2177_devp->si_prop,&si2177_devp->si_cmd_reply,&si2177_devp->si_common_reply);
		if(err_code)
        {
                printk("[si2177..]%s init si2177 error.\n",__func__);
                return err_code;
        }
        return 0;
}
static int si2177_leave_mode(struct aml_fe *fe, int mode)
{
        int err_code;
	si2177_devp->parm.frequency=0;
	si2177_devp->parm.std=0;
        err_code = si2177_power_down(&si2177_devp->tuner_client, &si2177_devp->si_cmd_reply);
        if(err_code)
        {
                printk("[si2177..]%s power down si2177 error.\n",__func__);
                return err_code;
        }
        return 0;
}

static int si2177_suspend(struct aml_fe_dev *dev)
{
	int err_code;
	si2177_devp->parm.frequency=0;
	si2177_devp->parm.std=0;
	err_code = si2177_power_down(&si2177_devp->tuner_client, &si2177_devp->si_cmd_reply);
	 if(err_code)
        {
        printk("[si2177..]%s si2177 standby mode is err.\n",__func__);
        return err_code;
        }
	 return 0;

}
static int si2177_resume(struct aml_fe_dev *dev)
{
	int err_code;
	amlogic_gpio_request(dev->reset_gpio,TUNER_POWER_GPIO_OWNER);//lock

   amlogic_gpio_direction_output(dev->reset_gpio,0,TUNER_POWER_GPIO_OWNER);
   udelay(400);
   amlogic_gpio_direction_output(dev->reset_gpio,1,TUNER_POWER_GPIO_OWNER);

	amlogic_gpio_free(dev->reset_gpio,TUNER_POWER_GPIO_OWNER); //free

        err_code = si2177_init(&si2177_devp->tuner_client,&si2177_devp->si_cmd_reply,&si2177_devp->si_common_reply);
        err_code = si2177_configure(&si2177_devp->tuner_client,&si2177_devp->si_prop,&si2177_devp->si_cmd_reply,&si2177_devp->si_common_reply);
        if(err_code)
        {
                printk("[si2177..]%s resume si2177 error.\n",__func__);
                return err_code;
        }
        return 0;
}
static struct aml_fe_drv si2177_tuner_drv = {
        .name    = "si2177_tuner",
        .capability = AM_FE_ANALOG|AM_FE_QPSK|AM_FE_QAM|AM_FE_ATSC|AM_FE_OFDM|AM_FE_DTMB,
        .id      = AM_TUNER_SI2177,
        .get_ops = si2177_tuner_get_ops,
        .init    = si2177_tuner_fe_init,
        .enter_mode = si2177_enter_mode,
        .leave_mode = si2177_leave_mode,
        .suspend = si2177_suspend,
        .resume = si2177_resume,
};

static int si2177_analog_get_afc(struct dvb_frontend *fe, s32 *afc)
{
        if(si2177_atv_status(&si2177_devp->tuner_client, SI2177_ATV_STATUS_CMD_INTACK_OK, &si2177_devp->si_cmd_reply)!=0)
        {
                pr_info("[si2177..]%s: get si2177 atv status error.\n",__func__);
				*afc=0;
                return -ERROR;
        }
        else
                *afc = si2177_devp->si_cmd_reply.atv_status.afc_freq;
		return 0;
}
static int si2177_analog_get_snr(struct dvb_frontend *fe)
{
        if(si2177_atv_status(&si2177_devp->tuner_client, SI2177_ATV_STATUS_CMD_INTACK_OK, &si2177_devp->si_cmd_reply)!=0)
        {
                pr_info("[si2177..]%s: get si2177 atv status error.\n",__func__);
                return -ERROR;
        }
        else
                return si2177_devp->si_cmd_reply.atv_status.video_snr;
}
//tuner lock status & demod lock status should be same in silicon tuner
static void si2177_get_status(struct dvb_frontend *fe, void *stat)
{
        fe_status_t *status = (fe_status_t*)stat;
        if(!status)
        {
                printk("[si2177..]%s: null pointer error.\n",__func__);
                return;
        }
        if(si2177_atv_status(&si2177_devp->tuner_client, SI2177_ATV_STATUS_CMD_INTACK_OK, &si2177_devp->si_cmd_reply))
        {
                pr_info("[si2177..]%s:get si2177 atv status error.\n",__func__);
                return;
        }
        else
        {
                if(si2177_devp->si_cmd_reply.atv_status.chl)
                        *status = FE_HAS_LOCK;
                else
                        *status = FE_TIMEDOUT;
        }
}
//tuner lock status & demod lock status should be same in silicon tuner
static void si2177_get_pll_status(struct dvb_frontend *fe, void *stat)
{
        fe_status_t *status = (fe_status_t*)stat;
        if(!status)
        {
                printk("[si2177..]%s: null pointer error.\n",__func__);
                return;
        }
        if(si2177_atv_status(&si2177_devp->tuner_client, SI2177_ATV_STATUS_CMD_INTACK_OK, &si2177_devp->si_cmd_reply))
        {
                pr_info("[si2177..]%s:get si2177 atv status error.\n",__func__);
                return;
        }
        else
        {
                if(si2177_devp->si_cmd_reply.atv_status.pcl&&si2177_devp->si_cmd_reply.atv_status.dl)
                        *status = FE_HAS_LOCK;
                else
                        *status = FE_TIMEDOUT;
        }
}

static int si2177_analog_get_atv_status(struct dvb_frontend *dvb_fe, atv_status_t *atv_status)
{
		si2177_cmdreplyobj_t  *si_cmd_reply = &si2177_devp->si_cmd_reply;
//get the tuner_status to give cvd to use
	if(si2177_tuner_status(&si2177_devp->tuner_client, 0, si_cmd_reply)!=0)
        {
                printk("[si2177..]%s:the patch:get si2177 tuner status error.\n",__func__);
                return -ERROR;
        }
        if(!atv_status)
        {
                printk("[si2177..]%s: null pointer error.\n",__func__);
                return -ERROR;
        }
        if(si2177_atv_status(&si2177_devp->tuner_client, SI2177_ATV_STATUS_CMD_INTACK_OK, &si2177_devp->si_cmd_reply)!=0)
        {
                pr_info("[si2177..]%s:get si2177 atv status error.\n",__func__);
                return -ERROR;
        }
        else
        {
                atv_status->afc = si2177_devp->si_cmd_reply.atv_status.afc_freq;
                atv_status->snr =si2177_devp->si_cmd_reply.atv_status.video_snr;
                atv_status->atv_lock = si2177_devp->si_cmd_reply.atv_status.chl;
                atv_status->audmode = si2177_devp->si_cmd_reply.atv_status.audio_demod_mode;
                atv_status->std &= 0;
                if(si2177_devp->si_cmd_reply.atv_status.color);
        //                atv_status->std |= V4L2_COLOR_STD_SECAM;
                else;
  //                      atv_status->std = V4L2_COLOR_STD_PAL;
                switch(si2177_devp->si_cmd_reply.atv_status.video_sys)
                {
                        case 0:
                                atv_status->std |=V4L2_STD_B;
                                break;
                        case 1:
                                atv_status->std |=V4L2_STD_GH;
                                break;
                        case 2:
                                atv_status->std |=V4L2_STD_PAL_M;
                                break;
                        case 3:
                                atv_status->std |=V4L2_STD_PAL_N;
                                break;
                        case 4:
                                atv_status->std |=V4L2_STD_PAL_I;
                                break;
                        case 5:
                                atv_status->std |=V4L2_STD_DK;
                                break;
                        case 6:
                                atv_status->std |=V4L2_STD_SECAM_L;
                                break;
                        case 7:
                                atv_status->std |=V4L2_STD_SECAM_LC;
                                break;
                        default:
                                break;
                }

        }
//it is the si2177_debug==2,the printk is valid
		ssiprintk("[si2177..]%s afc %d,snr %d,atv lock %d audmode %d,std color %s audio %s.\n",__func__,atv_status->afc, atv_status->snr,
                        atv_status->atv_lock, atv_status->audmode, v4l2_std_to_str(atv_status->std&0xff000000),v4l2_std_to_str(atv_status->std&0xffffff));
        return 0;
}

static int si2177_analog_get_ops(struct aml_fe_dev *dev, int mode, void *ops)
{

        struct analog_demod_ops *si2177_analog_ops = (struct analog_demod_ops*)ops;
        if(!ops)
        {
                printk("[si2177..]%s null pointer error.\n",__func__);
                return -ERROR;
        }
        si2177_analog_ops->get_afc               = si2177_analog_get_afc;
        si2177_analog_ops->get_snr               = si2177_analog_get_snr;
        si2177_analog_ops->get_status          = si2177_get_status;
	si2177_analog_ops->get_pll_status          = si2177_get_pll_status;
        si2177_analog_ops->get_atv_status   = si2177_analog_get_atv_status;
        return 0;
}

static struct aml_fe_drv si2177_analog_drv = {
        .name    = "si2177_atv_demod",
        .capability = AM_FE_ANALOG,
        .id      = AM_ATV_DEMOD_SI2177 ,//2177 bsp follow the si2176
        .get_ops = si2177_analog_get_ops
};

static int __init si2177_tuner_init(void)
{
        int ret = 0;
        si2177_devp = kmalloc(sizeof(struct si2177_device_s), GFP_KERNEL);

        if(!si2177_devp)
        {
                pr_info("[si2177..] %s:allocate memory error,no enough memory for struct si2177_dev_s.\n",__func__);
                return -ENOMEM;
        }
        memset(si2177_devp, 0, sizeof(struct si2177_device_s));

        si2177_devp->clsp = class_create(THIS_MODULE,TUNER_DEVICE_NAME);
        if(!si2177_devp->clsp)
        {
                pr_info("[si2177..]%s:create class error.\n",__func__);
                kfree(si2177_devp);
                si2177_devp = NULL;
                return PTR_ERR(si2177_devp->clsp);
        }
        ret = class_create_file(si2177_devp->clsp, &class_attr_si2177_debug);
        if(ret)
                pr_err("[si2177]%s create si2177 class file error.\n",__func__);
        /*initialize the tuner common struct and register*/
        aml_register_fe_drv(AM_DEV_TUNER, &si2177_tuner_drv);
        aml_register_fe_drv(AM_DEV_ATV_DEMOD, &si2177_analog_drv);
        printk("[si2177..]%s.\n",__func__);
        return 0;
}

static void __exit si2177_tuner_exit(void)
{
        class_destroy(si2177_devp->clsp);
        kfree(si2177_devp);
        aml_unregister_fe_drv(AM_DEV_ATV_DEMOD, &si2177_analog_drv);
        aml_unregister_fe_drv(AM_DEV_TUNER, &si2177_tuner_drv);
        pr_info("[si2177..]%s: driver removed ok.\n",__func__);
}

MODULE_AUTHOR("kele.bai <kele.bai@amlogic.com>");
MODULE_DESCRIPTION("si2177 tuner device driver");
MODULE_LICENSE("GPL");

fs_initcall(si2177_tuner_init);
module_exit(si2177_tuner_exit);

