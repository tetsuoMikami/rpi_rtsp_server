//==============================================================================
//! @brief RTSP server using Gstreamer
//! @author		T.Mikami
//! @copyright	T.Mikami
//! @since		2024/11/01
//==============================================================================
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include "rtsp_stream_server.h"

//======================================
//
//======================================
#if defined(USE_FUNC_OVERLAY_TIMESTAMP)
#define GST_INST_TIMESTAMP	"textoverlay name=text_overlay halignment=right valignment=bottom font-desc=\"Monospace, 12\" shaded-background=false ! "
#else
#define GST_INST_TIMESTAMP
#endif


//==================================================================================================
//! @brief			タイムスタンプ更新用の関数
//! @param[in]		user_data
//! @retval			TRUE
//! @note			---
//==================================================================================================
static gboolean update_time_overlay(gpointer user_data)
{
	XRtspServerFactory *factory = (XRtspServerFactory *)user_data;

	if (factory->text_overlay) {
		time_t now = time(NULL);
		struct tm *local = localtime(&now);

#if defined(USE_FUNC_OVERLAY_TIMESTAMP_TIME_ONLY)
		gchar time_str[9];
		strftime(time_str, sizeof(time_str), "%H:%M:%S", local);
#else
		gchar time_str[20];
		strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);
#endif

		// テキストオーバーレイに時刻を設定
		g_object_set(factory->text_overlay, "text", time_str, NULL);
	}

	return TRUE; // 継続して呼び出すためTRUEを返す
}

//==================================================================================================
//! @brief			メディア設定のコールバック関数
//! @param[in]		factory
//! @param[in]		media
//! @param[in]		user_data
//! @note			---
//==================================================================================================
static void media_configure(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data)
{
//	g_print("Media configure called for RTSP stream.\n");

	XRtspServerFactory *x_factory = (XRtspServerFactory *)factory;
	GstElement *pipeline = gst_rtsp_media_get_element(media);

	if (pipeline) {
		x_factory->text_overlay = gst_bin_get_by_name_recurse_up(GST_BIN(pipeline), "text_overlay");
		gst_object_unref(pipeline);
	}
}

//==================================================================================================
//! @brief			クラスの初期化関数
//! @param[in]		klass
//! @note			関数はG_DEFINE_TYPEマクロによって自動的に呼び出される
//==================================================================================================
static void xrtsp_server_factory_class_init(XRtspServerFactoryClass* klass)
{
	// 必要な設定があればここで行う
}

//==================================================================================================
//! @brief			インスタンスの初期化関数
//! @param[in]		factory
//! @param[out]		factory
//! @note			関数はG_DEFINE_TYPEマクロによって自動的に呼び出される
//==================================================================================================
static void xrtsp_server_factory_init(XRtspServerFactory* factory)
{
	factory->device = g_strdup("/dev/video0");
	factory->width = 1280;
	factory->height = 720;
	factory->framerate = 15;
	factory->gop_size = 15;
	factory->bitrate = 1000000;
	factory->text_overlay = NULL;

	gst_rtsp_media_factory_set_shared(GST_RTSP_MEDIA_FACTORY(factory), TRUE);

	gchar *pipeline_desc = g_strdup_printf(
		"( v4l2src device=%s do-timestamp=true ! image/jpeg,width=%d,height=%d,framerate=%d/1 ! "
		"jpegparse ! "
		"v4l2jpegdec ! video/x-raw,framerate=%d/1,format=I420 ! "
		GST_INST_TIMESTAMP
		"v4l2h264enc "
		"extra-controls=\"controls, "
		"video_b_frames=0, "
		"h264_profile=4, "
		"video_gop_size=%d, "
		"h264_i_frame_period=%d, "
		"repeat_sequence_header=1, "
		"h264_minimum_qp_value=26, "
		"video_bitrate=%d\" ! "
		"video/x-h264,"
		"stream-format=byte-stream,"
		"alignment=au,profile=high,"
		"level=(string)4.2 ! "
		"h264parse ! "
		"rtph264pay name=pay0 pt=96 )",
		factory->device, 
		factory->width, 
		factory->height, 
		factory->framerate,
		factory->framerate, 
		factory->gop_size, 
		factory->gop_size, 
		factory->bitrate
	);

	gst_rtsp_media_factory_set_launch(GST_RTSP_MEDIA_FACTORY(factory), pipeline_desc);
	g_free(pipeline_desc);
}

//==================================================================================================
//! @brief			RTSP server main
//! @param[in]		argc
//! @param[in]		argv[]
//! @retval			0 
//! @note			---
//==================================================================================================
int main(int argc, char* argv[])
{
	gst_init(&argc, &argv);

	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	GstRTSPServer *server = gst_rtsp_server_new();
	gst_rtsp_server_set_service(server, DEFAULT_SERVER_PORT);

	GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
	XRtspServerFactory *factory = g_object_new(xrtsp_server_factory_get_type(), NULL);
	gst_rtsp_mount_points_add_factory(mounts, "/main", GST_RTSP_MEDIA_FACTORY(factory));

	g_signal_connect(factory, "media-configure", (GCallback)media_configure, NULL);

	// 1秒ごとにタイムスタンプを更新
	g_timeout_add_seconds(1, (GSourceFunc)update_time_overlay, factory);

	gst_rtsp_server_attach(server, NULL);

	g_print("RTSP server is ready at rtsp://127.0.0.1:%s/main\n", DEFAULT_SERVER_PORT);
	g_main_loop_run(loop);

	g_object_unref(server);
	g_main_loop_unref(loop);

	return 0;
}
