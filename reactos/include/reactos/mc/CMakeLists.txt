
list(APPEND ANSI_SOURCE
    bugcodes.mc)

list(APPEND UNICODE_SOURCE
    errcodes.mc
    neteventmsg.mc
    ntiologc.mc
    ntstatus.mc
    pciclass.mc)

list(APPEND UNICODE_SOURCE_REALLY
    sacmsg.mc)

add_message_headers(ANSI ${ANSI_SOURCE})
# FIXME: this needs testing before switching to unicode
add_message_headers(ANSI ${UNICODE_SOURCE})
add_message_headers(UNICODE ${UNICODE_SOURCE_REALLY})
