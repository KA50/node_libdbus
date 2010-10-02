#include "node_dbus_message.hpp"

#include <iostream>

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

Message::Message(DBusMessage * msg) :
    base(),
    message_(msg)
{
}

Message::~Message() {
    if (message_) {
        dbus_message_unref(message_);
        message_ = NULL;
    }
}

void
Message::Initialize(v8_utils::ObjectHandle exports) {
    base::Initialize("Message", New);

    property("serial", GetSerial);
    property("replySerial", GetReplySerial, SetReplySerial);

    exports["Message"] = function();
}

Message *
Message::New(DBusMessage * msg) {
    HandleScope scope;
    Local<Value> arg = External::New(msg);
    return Message::unwrap( Message::function()->NewInstance(1, & arg) );
}

Handle<Value>
Message::New(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }
    Message * msg;
    if ( args[0]->IsExternal()) {
        msg = new Message(static_cast<DBusMessage*>(External::Unwrap(args[0])));
    } else if ( args[0]->IsInt32()) {
        int type = args[0]->Int32Value();
        msg = new Message(dbus_message_new(type));
    } else {
       return throwTypeError("argument 1 must be an integer (DBusMessageType)");
    }

    msg->Wrap(args.This());
    return args.This();
}

Handle<Value>
Message::CreateMethodCall(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 4)) {
        return throwArgumentCountMismatchException(args, 4);
    }

    if ( ! args[0]->IsString()) {
        return throwTypeError("argument 1 must be a string (destination)");
    }
    String::Utf8Value destination(args[0]->ToString());

    if ( ! args[1]->IsString()) {
        return throwTypeError("argument 2 must be a string (path)");
    }
    String::Utf8Value path(args[1]->ToString());

    if ( ! args[2]->IsString()) {
        return throwTypeError("argument 3 must be a string (interface)");
    }
    String::Utf8Value interface(args[2]->ToString());

    if ( ! args[3]->IsString()) {
        return throwTypeError("argument 4 must be a string (method)");
    }
    String::Utf8Value method(args[3]->ToString());

    DBusMessage * msg = dbus_message_new_method_call(
              *destination
            , *path
            , *interface
            , *method
    );
    if ( ! msg) {
        return throwError("Out of memory");
    }
    Local<Value> arg(External::New(msg));
    return scope.Close(Message::function()->NewInstance(1, & arg));
}

#define NODE_DBUS_NOT_IMPLEMENTED(what) \
    std::cerr << #what << ": " << "not implemented (" << __FILE__ << ")" << std::endl

Handle<Value>
Message::CreateMethodReturn(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }
    if ( ! Message::HasInstance( args[0] )) {
        return throwTypeError("argument 1 must be an object (Message)");
    }
    Message * method_call_message = unwrap(args.Holder());

    DBusMessage * msg = dbus_message_new_method_return(method_call_message->message());
    if ( ! msg) {
        return throwError("Out of memory");
    }
    Local<Value> arg(External::New(msg));
    return scope.Close(Message::function()->NewInstance(1, & arg));
}

Handle<Value>
Message::CreateErrorMessage(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 3)) {
        return throwArgumentCountMismatchException(args, 3);
    }
    if ( ! Message::HasInstance( args[0] )) {
        return throwTypeError("argument 1 must be an object (Message)");
    }
    Message * reply_msg = unwrap(args.Holder());

    if ( ! args[1]->IsString()) {
        return throwTypeError("argument 2 must be a string (error_name)");
    }
    String::Utf8Value error_name(args[1]->ToString());

    if ( ! args[2]->IsString()) {
        return throwTypeError("argument 3 must be a string (error_message)");
    }
    String::Utf8Value error_message(args[2]->ToString());

    DBusMessage * msg = dbus_message_new_error(
              reply_msg->message()
            , *error_name
            , *error_message
    );
    if ( ! msg) {
        return throwError("Out of memory");
    }
    Local<Value> arg(External::New(msg));
    return scope.Close(Message::function()->NewInstance(1, & arg));
}

Handle<Value>
Message::CreateSignal(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 3)) {
        return throwArgumentCountMismatchException(args, 3);
    }

    if ( ! args[0]->IsString()) {
        return throwTypeError("argument 1 must be a string (path)");
    }
    String::Utf8Value path(args[0]->ToString());

    if ( ! args[1]->IsString()) {
        return throwTypeError("argument 2 must be a string (interface)");
    }
    String::Utf8Value interface(args[1]->ToString());

    if ( ! args[2]->IsString()) {
        return throwTypeError("argument 3 must be a string (name)");
    }
    String::Utf8Value name(args[2]->ToString());

    DBusMessage * msg = dbus_message_new_signal(
              *path
            , *interface
            , *name
    );
    if ( ! msg) {
        return throwError("Out of memory");
    }
    Local<Value> arg(External::New(msg));
    return scope.Close(Message::function()->NewInstance(1, & arg));
}

v8::Handle<v8::Value>
Message::GetSerial(v8::Local<v8::String> property,
        const v8::AccessorInfo &info)
{
    Message * msg = unwrap(info.Holder());
    return HandleScope().Close(
            Integer::NewFromUnsigned(dbus_message_get_serial(msg->message())));
}

v8::Handle<v8::Value>
Message::GetReplySerial(v8::Local<v8::String> property,
        const v8::AccessorInfo &info)
{
    Message * msg = unwrap(info.Holder());
    return HandleScope().Close(
            Integer::NewFromUnsigned(dbus_message_get_reply_serial(msg->message())));
}

void
Message::SetReplySerial(v8::Local<v8::String> property, v8::Local<v8::Value> value,
        const v8::AccessorInfo& info)
{
    HandleScope scope;
    Message * msg = unwrap(info.Holder());
    if ( ! value->IsUint32()) {
        throwTypeError("reply serial must be an unsigned integer");
        return;
    }
    dbus_message_set_reply_serial(msg->message(), value->Uint32Value());
}

} // end of namespace node_dbus