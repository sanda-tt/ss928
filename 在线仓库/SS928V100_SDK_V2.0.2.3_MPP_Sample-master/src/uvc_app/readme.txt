Compile:
	only compile uvc default, if need uac, please change the value of "UAC_COMPILE" into "y" in Makefile.
	$ make clean
	$ make


There are 4 data input mode to select:
1. OT_UVC_MPP_BIND_UVC: for venc stream or yuv frame, VENC bind UVC or VPSS/VI bind UVC, stream/yuv data flows in the kernel space;
2. OT_UVC_SEND_VENC_STREAM: user get VENC stream from venc channel, save or send out by network, then send venc stream to UVC by ss_mpi_uvc_send_stream();
3. OT_UVC_SEND_YUV_FRAME: user get YUV frame from VI/VPSS, then send it to UVC by ss_mpi_uvc_send_frame();
4. OT_UVC_SEND_USER_STREAM: user have private buffer that contains encoded stream, send it to UVC by ss_mpi_uvc_send_user_stream(), this is not for YUV frame, we can use VI/VPSS bind UVC for YUV frame.


Usage & Command Args
	Usage1: ./sample_uvc -h
		-h  --print help information
	Usage2: ./sample_uvc [DEVICE_COUNT] [DATA_MODE]
		DEVICE_COUNT: uvc device(also for uvc channel) count, should in [1, 2], should consistent with ConfigUVC script.
		DATA_MODE: uvc data mode, should in [0, 2], will be stored in uvc_media.c:g_data_input_mode.
			0 OT_UVC_MPP_BIND_UVC
			1 OT_UVC_SEND_VENC_STREAM/OT_UVC_SEND_YUV_FRAME
			2 OT_UVC_SEND_USER_STREAM
