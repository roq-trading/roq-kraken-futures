set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    crypto.cpp
    json_account_balances_and_margins.cpp
    json_book.cpp
    json_book_snapshot.cpp
    json_cancel_order.cpp
    json_cancel_all_orders.cpp
    json_edit_order.cpp
    json_fills.cpp
    json_fills_snapshot.cpp
    json_instruments.cpp
    json_instruments_item.cpp
    json_margin_account.cpp
    json_open_orders.cpp
    json_open_orders_snapshot.cpp
    json_open_positions.cpp
    json_order.cpp
    json_rest_error.cpp
    json_send_order.cpp
    json_ticker.cpp
    json_trade.cpp
    json_trade_snapshot.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  ${PROJECT_NAME}-json
  ${PROJECT_NAME}-tools
  roq-core::roq-core
  roq-io::roq-io
  roq-logging::roq-logging
  aklomp::base64
  Catch2::Catch2
  ${RT_LIBRARIES})

set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
