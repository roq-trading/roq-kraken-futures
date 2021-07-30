set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES
    json_account_balances_and_margins.cpp
    json_book.cpp
    json_book_snapshot.cpp
    json_cancel_all_orders_ack.cpp
    json_create_order_ack.cpp
    json_instruments.cpp
    json_instruments_item.cpp
    json_margin_account.cpp
    json_open_orders_snapshot.cpp
    json_open_orders.cpp
    json_order.cpp
    json_ticker.cpp
    json_trade.cpp
    json_trade_snapshot.cpp
    main.cpp)

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries("${TARGET_NAME}" "${PROJECT_NAME}-json" roq-logging::roq-logging gtest_main
                      ${RT_LIBRARIES})

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
