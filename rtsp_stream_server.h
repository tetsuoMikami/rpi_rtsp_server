//==============================================================================
//! @brief RTSP server using Gstreamer
//! @author		T.Mikami
//! @copyright	T.Mikami
//! @since		2024/11/01
//==============================================================================
#ifndef RTSP_STREAM_SERVER_H__
#define RTSP_STREAM_SERVER_H__

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

//======================================
// function select
//======================================
#define USE_FUNC_OVERLAY_TIMESTAMP					/* Overlaying time information */
	#undef USE_FUNC_OVERLAY_TIMESTAMP_TIME_ONLY		/* Do not display year, month, day */

//======================================
//
//======================================
typedef struct _XRtspServerFactory XRtspServerFactory;
typedef struct _XRtspServerFactoryClass XRtspServerFactoryClass;

#define xrtsp_server_factory_parent_class parent_class

struct _XRtspServerFactory {
	GstRTSPMediaFactory parent;
	gchar *device;
	gint width;
	gint height;
	gint framerate;
	gint gop_size;
	gint bitrate;
	GstElement *text_overlay;
};

struct _XRtspServerFactoryClass {
	GstRTSPMediaFactoryClass parent_class;
};

G_DEFINE_TYPE(XRtspServerFactory, xrtsp_server_factory, GST_TYPE_RTSP_MEDIA_FACTORY)

//======================================
//
//======================================
#define DEFAULT_SERVER_PORT "8554"


#endif /* RTSP_STREAM_SERVER_H__ */
