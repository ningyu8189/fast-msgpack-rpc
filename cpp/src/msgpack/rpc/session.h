//
// msgpack::rpc::session - MessagePack-RPC for C++
//
// Copyright (C) 2009-2010 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MSGPACK_RPC_SESSION_H__
#define MSGPACK_RPC_SESSION_H__

#include "types.h"
#include "address.h"
#include "message.h"
#include "loop.h"
#include "caller.h"
#include "impl_fwd.h"

namespace msgpack {
namespace rpc {


class session : public caller<session> {
public:
	session(shared_session pimpl) : m_pimpl(pimpl) { }
	~session() { }

	const address& get_address() const;

	const address& get_self_address() const;

	loop get_loop();

	void set_timeout(unsigned int sec);

	unsigned int get_timeout() const;

public:
	caller_with with(option opt)
	{
		return caller_with(this, opt);
	}

protected:
	template <typename Method, typename Parameter>
	future send_request(Method method,
			const Parameter& param, shared_zone msglife,
			option opt = option());

	future send_request_impl(msgid_t msgid, vrefbuffer* vbuf, shared_zone life, option opt);

	future send_request_impl(msgid_t msgid, sbuffer* sbuf, option opt);

	// FIXME notify

	friend class caller<session>;

protected:
	shared_session m_pimpl;

	inline loop& get_loop_ref();

private:
	msgid_t next_msgid();

private:
	session();
};


template <typename Method, typename Parameter>
inline future caller_with::send_request(Method method,
		const Parameter& param, shared_zone msglife,
		option opt)
{
	return m_session->send_request(method, param, msglife, opt);
}


template <typename Method, typename Parameter>
future session::send_request(Method method,
		const Parameter& param, shared_zone msglife,
		option opt)
{
	msgid_t msgid = next_msgid();
	msg_request<Method, Parameter> msgreq(method, param, msgid);

	if(msglife) {
		msgpack::vrefbuffer* vbuf = msglife->allocate<msgpack::vrefbuffer>();
		msgpack::pack(vbuf, msgreq);
		return send_request_impl(msgid, vbuf, msglife, opt);

	} else {
		msgpack::sbuffer sbuf;
		msgpack::pack(sbuf, msgreq);
		return send_request_impl(msgid, &sbuf, opt);
	}
}


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/session.h */

