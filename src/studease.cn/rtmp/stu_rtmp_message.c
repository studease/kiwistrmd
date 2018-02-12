/*
 * stu_rtmp_message.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_str_t  STU_RTMP_CMD_CONNECT       = stu_string("connect");
stu_str_t  STU_RTMP_CMD_CLOSE         = stu_string("close");
stu_str_t  STU_RTMP_CMD_CREATE_STREAM = stu_string("createStream");
stu_str_t  STU_RTMP_CMD_RESULT        = stu_string("_result");
stu_str_t  STU_RTMP_CMD_ERROR         = stu_string("_error");

stu_str_t  STU_RTMP_CMD_PLAY          = stu_string("play");
stu_str_t  STU_RTMP_CMD_PLAY2         = stu_string("play2");
stu_str_t  STU_RTMP_CMD_DELETE_STREAM = stu_string("deleteStream");
stu_str_t  STU_RTMP_CMD_CLOSE_STREAM  = stu_string("closeStream");
stu_str_t  STU_RTMP_CMD_RECEIVE_AUDIO = stu_string("receiveAudio");
stu_str_t  STU_RTMP_CMD_RECEIVE_VIDEO = stu_string("receiveVideo");
stu_str_t  STU_RTMP_CMD_PUBLISH       = stu_string("publish");
stu_str_t  STU_RTMP_CMD_SEEK          = stu_string("seek");
stu_str_t  STU_RTMP_CMD_PAUSE         = stu_string("pause");
stu_str_t  STU_RTMP_CMD_ON_STATUS     = stu_string("onStatus");

stu_str_t  STU_RTMP_SET_BUFFER_LENGTH = stu_string("setBufferLength");
stu_str_t  STU_RTMP_SET_DATA_FRAME    = stu_string("@setDataFrame");
stu_str_t  STU_RTMP_CLEAR_DATA_FRAME  = stu_string("@clearDataFrame");
stu_str_t  STU_RTMP_AUDIO_FRAME       = stu_string("audioFrame");
stu_str_t  STU_RTMP_VIDEO_FRAME       = stu_string("videoFrame");

stu_str_t  STU_RTMP_LEVEL_ERROR   = stu_string("error");
stu_str_t  STU_RTMP_LEVEL_STATUS  = stu_string("status");
stu_str_t  STU_RTMP_LEVEL_WARNING = stu_string("warning");

stu_str_t  STU_RTMP_CODE_NETCONNECTION_CALL_FAILED         = stu_string("NetConnection.Call.Failed");
stu_str_t  STU_RTMP_CODE_NETCONNECTION_CONNECT_APPSHUTDOWN = stu_string("NetConnection.Connect.AppShutdown");
stu_str_t  STU_RTMP_CODE_NETCONNECTION_CONNECT_CLOSED      = stu_string("NetConnection.Connect.Closed");
stu_str_t  STU_RTMP_CODE_NETCONNECTION_CONNECT_FAILED      = stu_string("NetConnection.Connect.Failed");
stu_str_t  STU_RTMP_CODE_NETCONNECTION_CONNECT_IDLETIMEOUT = stu_string("NetConnection.Connect.IdleTimeout");
stu_str_t  STU_RTMP_CODE_NETCONNECTION_CONNECT_INVALIDAPP  = stu_string("NetConnection.Connect.InvalidApp");
stu_str_t  STU_RTMP_CODE_NETCONNECTION_CONNECT_REJECTED    = stu_string("NetConnection.Connect.Rejected");
stu_str_t  STU_RTMP_CODE_NETCONNECTION_CONNECT_SUCCESS     = stu_string("NetConnection.Connect.Success");

stu_str_t  STU_RTMP_CODE_NETSTREAM_BUFFER_EMPTY              = stu_string("NetStream.Buffer.Empty");
stu_str_t  STU_RTMP_CODE_NETSTREAM_BUFFER_FLUSH              = stu_string("NetStream.Buffer.Flush");
stu_str_t  STU_RTMP_CODE_NETSTREAM_BUFFER_FULL               = stu_string("NetStream.Buffer.Full");
stu_str_t  STU_RTMP_CODE_NETSTREAM_FAILED                    = stu_string("NetStream.Failed");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PAUSE_NOTIFY              = stu_string("NetStream.Pause.Notify");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PLAY_FAILED               = stu_string("NetStream.Play.Failed");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PLAY_FILESTRUCTUREINVALID = stu_string("NetStream.Play.FileStructureInvalid");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PLAY_PUBLISHNOTIFY        = stu_string("NetStream.Play.PublishNotify");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PLAY_RESET                = stu_string("NetStream.Play.Reset");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PLAY_START                = stu_string("NetStream.Play.Start");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PLAY_STOP                 = stu_string("NetStream.Play.Stop");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PLAY_STREAMNOTFOUND       = stu_string("NetStream.Play.StreamNotFound");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PLAY_UNPUBLISHNOTIFY      = stu_string("NetStream.Play.UnpublishNotify");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PUBLISH_BADNAME           = stu_string("NetStream.Publish.BadName");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PUBLISH_IDLE              = stu_string("NetStream.Publish.Idle");
stu_str_t  STU_RTMP_CODE_NETSTREAM_PUBLISH_START             = stu_string("NetStream.Publish.Start");
stu_str_t  STU_RTMP_CODE_NETSTREAM_RECORD_ALREADYEXISTS      = stu_string("NetStream.Record.AlreadyExists");
stu_str_t  STU_RTMP_CODE_NETSTREAM_RECORD_FAILED             = stu_string("NetStream.Record.Failed");
stu_str_t  STU_RTMP_CODE_NETSTREAM_RECORD_NOACCESS           = stu_string("NetStream.Record.NoAccess");
stu_str_t  STU_RTMP_CODE_NETSTREAM_RECORD_START              = stu_string("NetStream.Record.Start");
stu_str_t  STU_RTMP_CODE_NETSTREAM_RECORD_STOP               = stu_string("NetStream.Record.Stop");
stu_str_t  STU_RTMP_CODE_NETSTREAM_SEEK_FAILED               = stu_string("NetStream.Seek.Failed");
stu_str_t  STU_RTMP_CODE_NETSTREAM_SEEK_INVALIDTIME          = stu_string("NetStream.Seek.InvalidTime");
stu_str_t  STU_RTMP_CODE_NETSTREAM_SEEK_NOTIFY               = stu_string("NetStream.Seek.Notify");
stu_str_t  STU_RTMP_CODE_NETSTREAM_STEP_NOTIFY               = stu_string("NetStream.Step.Notify");
stu_str_t  STU_RTMP_CODE_NETSTREAM_UNPAUSE_NOTIFY            = stu_string("NetStream.Unpause.Notify");
stu_str_t  STU_RTMP_CODE_NETSTREAM_UNPUBLISH_SUCCESS         = stu_string("NetStream.Unpublish.Success");
stu_str_t  STU_RTMP_CODE_NETSTREAM_VIDEO_DIMENSIONCHANGE     = stu_string("NetStream.Video.DimensionChange");
