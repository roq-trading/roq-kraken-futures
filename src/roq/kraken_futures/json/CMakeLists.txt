set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES parser_public.cpp parser_private.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account_balances_and_margins.json
    alert.json
    book.json
    book_item.json
    book_snapshot.json
    challenge.json
    error.json
    event.json
    feed.json
    fill.json
    fill_type.json
    fills.json
    fills_snapshot.json
    heartbeat.json
    info.json
    instruments.json
    instruments_item.json
    margin_account.json
    open_orders.json
    open_orders_snapshot.json
    open_positions.json
    order.json
    order_type.json
    position.json
    product_id.json
    result.json
    side.json
    subscribed.json
    tag.json
    ticker.json
    trade.json
    trade_snapshot.json
    trade_type.json
    type.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/kraken_futures/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "h"
  SOURCES
  ${AUTOGEN_SCHEMAS})

set(GITIGNORE "${CMAKE_CURRENT_SOURCE_DIR}/.gitignore")

file(WRITE "${GITIGNORE}" "")

foreach(path ${AUTOGEN_SCHEMAS})
  get_filename_component(name ${path} NAME_WE)
  file(APPEND "${GITIGNORE}" "${name}.h\n${name}.cpp\n")
endforeach()

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  "roq/kraken_futures/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_library("${TARGET_NAME}" STATIC ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-json-headers)

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt)
