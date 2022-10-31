/***********************************************************************************************
					created: 		2019-05-13
					author:			chensong
					purpose:		msg_dipatch
************************************************************************************************/

#include "cmsg_dispatch.h"
#include "clog.h"
#include "cclient.h"
namespace chen {
	cmsg_dispatch::cmsg_dispatch()
	{
	}
	cmsg_dispatch::~cmsg_dispatch()
	{
	}
	bool cmsg_dispatch::init()
	{



		/*m_server_notification_protoo_msg_call.insert(
			std::make_pair(S2C_Login, &cclient::handler_Login));
		m_server_notification_protoo_msg_call.insert(std::make_pair(
			S2C_CreateRtc,
			&cclient::handler_create_webrtc_transport));
		m_server_notification_protoo_msg_call.insert(std::make_pair(
			S2C_RtcConnect,
			&cclient::handler_connect_webrtc_transport));

		m_server_notification_protoo_msg_call.insert(std::make_pair(
			S2C_RtcProduce,
			&cclient::handler_produce_webrtc_transport));

		m_server_notification_protoo_msg_call.insert(
			std::make_pair(S2C_RtpCapabilitiesUpdate,
				       &cclient::handler_rtp_capabilities));*/
		

		_register_msg_handler(S2C_Login, "S2C_Login",  &cclient ::handler_Login);
		_register_msg_handler(S2C_CreateRtc, "S2C_CreateRtc",  &cclient::handler_create_webrtc_transport);
		_register_msg_handler(
			S2C_RtcConnect, "S2C_RtcConnect",
			&cclient::handler_connect_webrtc_transport);
		 
		/// ///////////////////////////////////////////
		 
		_register_msg_handler(
			S2C_RtcProduce, "S2C_RtcProduce",
			&cclient::handler_produce_webrtc_transport);
		_register_msg_handler(
			S2C_RtpCapabilitiesUpdate, "S2C_RtpCapabilitiesUpdate",
				      &cclient::handler_rtp_capabilities);
		_register_msg_handler(S2C_JoinRoomUpdate, "S2C_JoinRoomUpdate",  
				      &cclient::handler_join_room_update);
 

		return true;
	}
	void cmsg_dispatch::destroy()
	{
		/*for (std::unordered_map < std::string, cmsg_handler*>::iterator iter = m_msg_handler_map.begin(); iter != m_msg_handler_map.end(); ++iter)
		{
			if (iter->second)
			{
				delete iter->second;
			}
		}
		m_msg_handler_map.clear();*/
	}
	cmsg_handler * cmsg_dispatch::get_msg_handler(uint16 msg_id)
	{
		if (static_cast<int> (msg_id) >= Msg_Client_Max  )
		{
			return NULL;
		}

		return &(m_msg_handler[msg_id]);
	}
	void cmsg_dispatch::_register_msg_handler(uint16 msg_id, const std::string & msg_name, handler_type handler)
	{
		if (static_cast<int> (msg_id) >= Msg_Client_Max || m_msg_handler[msg_id].handler)
		{
			ERROR_LOG("[%s] register msg error, msg_id = %u, msg_name = %s", __FUNCTION__, msg_id, msg_name.c_str());
			return;
		}

		m_msg_handler[msg_id].msg_name = msg_name;//   数据统计
		m_msg_handler[msg_id].handle_count = 0;
		m_msg_handler[msg_id].handler = handler;
	}


	cmsg_dispatch g_msg_dispatch;
} //namespace chen
