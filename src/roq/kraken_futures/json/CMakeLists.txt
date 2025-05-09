set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES map.cpp parser_public.cpp parser_private.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account_balances_and_margins.json
    alert.json
    book_item.json
    book.json
    book_snapshot.json
    cancel_all_after_ack.json
    cancel_all_after_ack_status.json
    cancel_all_orders.json
    cancel_all_status.json
    cancel_order.json
    cancel_status.json
    challenge.json
    edit_order.json
    edit_status.json
    error.json
    event.json
    feed.json
    fill.json
    fills.json
    fills_snapshot.json
    fill_type.json
    heartbeat.json
    info.json
    instruments_item.json
    instruments.json
    instrument_type.json
    margin_account.json
    margin_account_type.json
    open_orders.json
    open_orders_snapshot.json
    open_positions.json
    option_type.json
    order_event_item.json
    order_event.json
    order_event_order_type.json
    order_event_type.json
    order.json
    order_type.json
    position.json
    product_id.json
    reason.json
    rest_error_item.json
    rest_error.json
    result.json
    send_order.json
    send_status.json
    side.json
    status.json
    subscribed.json
    tag.json
    ticker.json
    trade.json
    trade_snapshot.json
    trade_type.json
    type.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen_cpp(
  OUTPUT
  AUTOGEN_SOURCES
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library(${TARGET_NAME} OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies(${TARGET_NAME} autogen-json-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-codec::roq-codec roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
