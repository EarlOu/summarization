#include "rpi/rpi_hardware/RpiHardwareEncoder.h"

#include <string>
using std::string;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

extern "C" {
#include "bcm_host.h"
#include "ilclient.h"
}

static void
print_def(OMX_PARAM_PORTDEFINITIONTYPE def)
{
   printf("Port %u: %s %u/%u %u %u %s,%s,%s %ux%u %ux%u @%u %u\n",
	  def.nPortIndex,
	  def.eDir == OMX_DirInput ? "in" : "out",
	  def.nBufferCountActual,
	  def.nBufferCountMin,
	  def.nBufferSize,
	  def.nBufferAlignment,
	  def.bEnabled ? "enabled" : "disabled",
	  def.bPopulated ? "populated" : "not pop.",
	  def.bBuffersContiguous ? "contig." : "not cont.",
	  def.format.video.nFrameWidth,
	  def.format.video.nFrameHeight,
	  def.format.video.nStride,
	  def.format.video.nSliceHeight,
	  def.format.video.xFramerate, def.format.video.eColorFormat);
}

RpiHardwareEncoder::RpiHardwareEncoder(): _init(false) {
    bcm_host_init();
}

void RpiHardwareEncoder::init(int w, int h, int fps, int out_fd) {
    if (_init) {
        fprintf(stderr, "call init() twice without release hardware\n");
        exit(EXIT_FAILURE);
    }
    if (_w % 32 != 0) {
        fprintf(stderr, "width should be multiple of 32\n");
        exit(EXIT_FAILURE);
    }
    if (_h % 16 != 0) {
        fprintf(stderr, "height should be multiple of 16\n");
        exit(EXIT_FAILURE);
    }
    _w = w;
    _h = h;
    _size = w * h * 4; // ABGR8888
    _fps = fps;
    _out_fd = out_fd;
    _init = true;
    _video_encode_context = NULL;
    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    OMX_PARAM_PORTDEFINITIONTYPE def;
    OMX_ERRORTYPE r;
    ILCLIENT_T *client;
    int status = 0;
    int framenumber = 0;
    FILE *outf;

    if ((client = ilclient_init()) == NULL) {
        fprintf(stderr, "Failed in ilclient_init()\n");
        exit(EXIT_FAILURE);
    }
    _client = (void*) client;

    r = OMX_Init();
    if (r != OMX_ErrorNone) {
        ilclient_destroy(client);
        fprintf(stderr, "Failed in OMX_Init(), %x", r);
        exit(EXIT_FAILURE);
    }

    // create _video_encode_context
    int res = ilclient_create_component(client, &_video_encode_context, "video_encode",
            (ILCLIENT_CREATE_FLAGS_T) ((int) ILCLIENT_DISABLE_ALL_PORTS |
            (int) ILCLIENT_ENABLE_INPUT_BUFFERS |
            (int) ILCLIENT_ENABLE_OUTPUT_BUFFERS));
    if (res != 0) {
        fprintf(stderr, "ilclient_create_component() for video_encode failed with %x!\n", r);
        exit(EXIT_FAILURE);
    }

    // get current settings of video_encode component from port 200
    memset(&def, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    def.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    def.nVersion.nVersion = OMX_VERSION;
    def.nPortIndex = 200;

    if (OMX_GetParameter
            (ILC_GET_HANDLE(_video_encode_context), OMX_IndexParamPortDefinition,
             &def) != OMX_ErrorNone) {
        fprintf(stderr, "%s:%d: OMX_GetParameter() for video_encode port 200 failed!\n",
                __FUNCTION__, __LINE__);
        exit(EXIT_FAILURE);
    }

    def.format.video.nFrameWidth = _w;
    def.format.video.nFrameHeight = _h;
    def.format.video.xFramerate = _fps << 16;
    def.format.video.nSliceHeight = def.format.video.nFrameHeight;
    def.format.video.nStride = def.format.video.nFrameWidth;
    def.format.video.eColorFormat = OMX_COLOR_Format32bitABGR8888;

    print_def(def);

    r = OMX_SetParameter(ILC_GET_HANDLE(_video_encode_context),
            OMX_IndexParamPortDefinition, &def);
    if (r != OMX_ErrorNone) {
        fprintf(stderr, "%s:%d: OMX_SetParameter() for video_encode port 200 failed with %x!\n",
             __FUNCTION__, __LINE__, r);
        exit(EXIT_FAILURE);
    }

    memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nVersion.nVersion = OMX_VERSION;
    format.nPortIndex = 201;
    format.eCompressionFormat = OMX_VIDEO_CodingAVC;

    printf("OMX_SetParameter for video_encode:201...\n");
    r = OMX_SetParameter(ILC_GET_HANDLE(_video_encode_context),
            OMX_IndexParamVideoPortFormat, &format);
    if (r != OMX_ErrorNone) {
        fprintf(stderr, "%s:%d: OMX_SetParameter() for video_encode port 201 failed with %x!\n",
             __FUNCTION__, __LINE__, r);
        exit(EXIT_FAILURE);
    }

    OMX_VIDEO_PARAM_BITRATETYPE bitrateType;
    // set current bitrate to 1Mbit
    memset(&bitrateType, 0, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
    bitrateType.nSize = sizeof(OMX_VIDEO_PARAM_BITRATETYPE);
    bitrateType.nVersion.nVersion = OMX_VERSION;
    bitrateType.eControlRate = OMX_Video_ControlRateVariable;
    bitrateType.nTargetBitrate = 1000000;
    bitrateType.nPortIndex = 201;
    r = OMX_SetParameter(ILC_GET_HANDLE(_video_encode_context),
            OMX_IndexParamVideoBitrate, &bitrateType);
    if (r != OMX_ErrorNone) {
        fprintf(stderr, "%s:%d: OMX_SetParameter() for bitrate for video_encode port 201 failed with %x!\n",
             __FUNCTION__, __LINE__, r);
        exit(EXIT_FAILURE);
    }

    // get current bitrate
    memset(&bitrateType, 0, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
    bitrateType.nSize = sizeof(OMX_VIDEO_PARAM_BITRATETYPE);
    bitrateType.nVersion.nVersion = OMX_VERSION;
    bitrateType.nPortIndex = 201;

    if (OMX_GetParameter(ILC_GET_HANDLE(_video_encode_context), OMX_IndexParamVideoBitrate,
             &bitrateType) != OMX_ErrorNone) {
        fprintf(stderr, "%s:%d: OMX_GetParameter() for video_encode for bitrate port 201 failed!\n",
                __FUNCTION__, __LINE__);
        exit(EXIT_FAILURE);
    }
    printf("Current Bitrate=%u\n",bitrateType.nTargetBitrate);

    printf("encode to idle...\n");
    if (ilclient_change_component_state(_video_encode_context, OMX_StateIdle) == -1) {
        fprintf(stderr, "%s:%d: ilclient_change_component_state(video_encode, OMX_StateIdle) failed",
             __FUNCTION__, __LINE__);
        exit(EXIT_FAILURE);
    }

    printf("enabling port buffers for 200...\n");
    if (ilclient_enable_port_buffers(_video_encode_context, 200, NULL, NULL, NULL) != 0) {
        fprintf(stderr, "enabling port buffers for 200 failed!\n");
        exit(EXIT_FAILURE);
    }

    printf("enabling port buffers for 201...\n");
    if (ilclient_enable_port_buffers(_video_encode_context, 201, NULL, NULL, NULL) != 0) {
        fprintf(stderr, "enabling port buffers for 201 failed!\n");
        exit(EXIT_FAILURE);
    }

    printf("encode to executing...\n");
    ilclient_change_component_state(_video_encode_context, OMX_StateExecuting);
}

void RpiHardwareEncoder::encode(uint8_t* frame) {
    if (!_init) {
        fprintf(stderr, "call encode() before calling init()\n");
        exit(EXIT_FAILURE);
    }
    OMX_BUFFERHEADERTYPE *buf;
    OMX_BUFFERHEADERTYPE *out;
    int r;
    buf = ilclient_get_input_buffer(_video_encode_context, 200, 1);
    if (buf == NULL) {
        fprintf(stderr, "no buffers for me!\n");
        exit(EXIT_FAILURE);
    }
    memcpy(buf->pBuffer, frame, _size);
    buf->nFilledLen=_size;

    if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(_video_encode_context), buf) !=
            OMX_ErrorNone) {
        fprintf(stderr, "Error emptying buffer!\n");
        exit(EXIT_FAILURE);
    }

    out = ilclient_get_output_buffer(_video_encode_context, 201, 1);

    r = (int) OMX_FillThisBuffer(ILC_GET_HANDLE(_video_encode_context), out);
    if (r != OMX_ErrorNone) {
        fprintf(stderr, "Error filling buffer: %x\n", r);
        exit(EXIT_FAILURE);
    }

    if (out != NULL) {
        // if (out->nFlags & OMX_BUFFERFLAG_CODECCONFIG) {
        //     int i;
        //     for (i = 0; i < out->nFilledLen; i++) printf("%x ", out->pBuffer[i]);
        //     printf("\n");
        // }

        int n = 0;
        while (n < out->nFilledLen) {
            r = write(_out_fd, out->pBuffer + n, out->nFilledLen - n);
            if (r < 0) {
                perror("Error writing result buffer");
                exit(EXIT_FAILURE);
            }
            n += r;
        }
        out->nFilledLen = 0;
    }
}

void RpiHardwareEncoder::release() {
    if (!_init) return;
    printf("disabling port buffers for 200 and 201...\n");
    ilclient_disable_port_buffers(_video_encode_context, 200, NULL, NULL, NULL);
    ilclient_disable_port_buffers(_video_encode_context, 201, NULL, NULL, NULL);

    COMPONENT_T *list[5];
    memset(list, 0, sizeof(list));
    list[0] = _video_encode_context;
    ilclient_state_transition(list, OMX_StateIdle);
    ilclient_state_transition(list, OMX_StateLoaded);
    ilclient_cleanup_components(list);

    OMX_Deinit();
    ilclient_destroy((ILCLIENT_T* )_client);
}
