W0528 14:16:31.340488 1135147 L0 drop_copy.cpp:376] DEBUG open_orders={feed=OPEN_ORDERS, order_id="", cli_ord_id="", order={instrument="PF_SOLUSD", time=1748434591319ms, last_update_time=1748434591319ms, qty=1, filled=0, limit_price=174.6, stop_price=0, type=LIMIT, order_id="9f04ea45-da4d-45a1-a40c-8ca38151ab03", cli_ord_id="pQAFV5WVrB0AAQAAAAAA", direction=1, reduce_only=false}, is_cancel=false, reason=NEW_PLACED_ORDER_BY_USER}
W0528 14:16:31.343244 1135147 L0 order_entry.cpp:262] DEBUG body="{"result":"success","sendStatus":{"cliOrdId":"pQAFV5WVrB0AAQAAAAAA","status":"placed","receivedTime":"2025-05-28T12:16:31.319813Z","orderEvents":[{"order":{"orderId":"9f04ea45-da4d-45a1-a40c-8ca38151ab03","cliOrdId":"pQAFV5WVrB0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":1,"filled":0,"limitPrice":174.60,"reduceOnly":false,"timestamp":"2025-05-28T12:16:31.319Z","lastUpdateTimestamp":"2025-05-28T12:16:31.319Z"},"reducedQuantity":null,"type":"PLACE"}],"order_id":"9f04ea45-da4d-45a1-a40c-8ca38151ab03"},"serverTime":"2025-05-28T12:16:31.319Z"}"






W0528 14:16:38.040615 1135147 L0 drop_copy.cpp:376] DEBUG open_orders={feed=OPEN_ORDERS, order_id="", cli_ord_id="", order={instrument="PF_SOLUSD", time=1748434591319ms, last_update_time=1748434597999ms, qty=0.8600000000000001, filled=0.14, limit_price=174.6, stop_price=0, type=LIMIT, order_id="9f04ea45-da4d-45a1-a40c-8ca38151ab03", cli_ord_id="pQAFV5WVrB0AAQAAAAAA", direction=1, reduce_only=false}, is_cancel=false, reason=PARTIAL_FILL}
W0528 14:16:38.040685 1135147 L0 order_state.cpp:777] DEBUG: [OMS:127449535831] override remaining_quantity=0.7200000000000001, (quantity=0.8600000000000001, traded_quantity=0.14)
W0528 14:16:38.040877 1135147 L0 drop_copy.cpp:432] DEBUG fills={feed=FILLS, username="cc34f46b-bc3b-47a2-a7fa-e19e34e376f0", fills=[{instrument="PF_SOLUSD", time=1748434597999ms, price=174.6, seq=24, buy=false, qty=0.14, order_id="9f04ea45-da4d-45a1-a40c-8ca38151ab03", cli_ord_id="pQAFV5WVrB0AAQAAAAAA", fill_id="2a2b437e-0e1c-44cb-ac2a-5c53b51d4667", fill_type=MAKER, fee_paid=0.004888800000000001, fee_currency="USD", taker_order_type="ioc", remaining_order_qty=0.8600000000000001, order_type="lmt"}]}
W0528 14:16:38.204198 1135147 L0 drop_copy.cpp:376] DEBUG open_orders={feed=OPEN_ORDERS, order_id="", cli_ord_id="", order={instrument="PF_SOLUSD", time=1748434591319ms, last_update_time=1748434598183ms, qty=0, filled=1, limit_price=174.6, stop_price=0, type=LIMIT, order_id="9f04ea45-da4d-45a1-a40c-8ca38151ab03", cli_ord_id="pQAFV5WVrB0AAQAAAAAA", direction=1, reduce_only=false}, is_cancel=true, reason=FULL_FILL}
W0528 14:16:38.204745 1135147 L0 drop_copy.cpp:432] DEBUG fills={feed=FILLS, username="cc34f46b-bc3b-47a2-a7fa-e19e34e376f0", fills=[{instrument="PF_SOLUSD", time=1748434598183ms, price=174.6, seq=25, buy=false, qty=0.8600000000000001, order_id="9f04ea45-da4d-45a1-a40c-8ca38151ab03", cli_ord_id="pQAFV5WVrB0AAQAAAAAA", fill_id="6c0b92a4-a7fa-479f-8d5a-13fa2f099167", fill_type=MAKER, fee_paid=0.030031200000000004, fee_currency="USD", taker_order_type="ioc", remaining_order_qty=0, order_type="lmt"}]}




D0528 14:17:29.185913 1135147 L0 utils.cpp:165] DEBUG: quantity=1, precision={increment=1, precision=_0}, value=1, step=1, residual=0
D0528 14:17:29.185990 1135147 L0 utils.cpp:196] DEBUG: price=174.5, precision={increment=0.010000000000000002, precision=_2}, value=17450, step=1, residual=0
W0528 14:17:29.256714 1135147 L0 drop_copy.cpp:432] DEBUG fills={feed=FILLS, username="cc34f46b-bc3b-47a2-a7fa-e19e34e376f0", fills=[{instrument="PF_SOLUSD", time=1748434649233ms, price=174.47, seq=26, buy=true, qty=1, order_id="9f04ea9e-38f2-41e1-b1e0-617656e5ae82", cli_ord_id="qgAFWJWVrB0AAQAAAAAA", fill_id="014b3cdd-7500-4be5-a238-513ffb8b6b1a", fill_type=TAKER, fee_paid=0.08723500000000003, fee_currency="USD", taker_order_type="lmt", remaining_order_qty=0, order_type="lmt"}]}
W0528 14:17:29.257475 1135147 L0 order_entry.cpp:262] DEBUG body="{"result":"success","sendStatus":{"cliOrdId":"qgAFWJWVrB0AAQAAAAAA","status":"placed","receivedTime":"2025-05-28T12:17:29.233781Z","orderEvents":[{"executionId":"014b3cdd-7500-4be5-a238-513ffb8b6b1a","price":174.47,"amount":1,"orderPriorEdit":null,"orderPriorExecution":{"orderId":"9f04ea9e-38f2-41e1-b1e0-617656e5ae82","cliOrdId":"qgAFWJWVrB0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"buy","quantity":1,"filled":0,"limitPrice":174.50,"reduceOnly":false,"timestamp":"2025-05-28T12:17:29.233Z","lastUpdateTimestamp":"2025-05-28T12:17:29.233Z"},"takerReducedQuantity":null,"type":"EXECUTION"}],"order_id":"9f04ea9e-38f2-41e1-b1e0-617656e5ae82"},"serverTime":"2025-05-28T12:17:29.234Z"}"








W0528 14:56:25.123966 1140091 L0 drop_copy.cpp:432] DEBUG fills={feed=FILLS, username="cc34f46b-bc3b-47a2-a7fa-e19e34e376f0", fills=[{instrument="PF_SOLUSD", time=1748436985105ms, price=174.32, seq=32, buy=false, qty=3, order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3", cli_ord_id="2gAFhnXarR0AAQAAAAAA", fill_id="5c58ccf8-22f5-4dbc-8f77-ff1740244a44", fill_type=TAKER, fee_paid=0.26148000000000005, fee_currency="USD", taker_order_type="lmt", remaining_order_qty=0, order_type="lmt"}]}
W0528 14:56:25.124081 1140091 L0 drop_copy.cpp:376] DEBUG open_orders={feed=OPEN_ORDERS, order_id="", cli_ord_id="", order={instrument="PF_SOLUSD", time=1748436703750ms, last_update_time=1748436985105ms, qty=0, filled=3, limit_price=174.3, stop_price=0, type=LIMIT, order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3", cli_ord_id="2gAFhnXarR0AAQAAAAAA", direction=1, reduce_only=false}, is_cancel=true, reason=FULL_FILL}
W0528 14:56:25.126006 1140091 L0 order_entry.cpp:369] DEBUG body="{"result":"success","serverTime":"2025-05-28T12:56:25.106Z","editStatus":{"status":"filled","orderId":"9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3","receivedTime":"2025-05-28T12:56:25.106Z","orderEvents":[{"executionId":"5c58ccf8-22f5-4dbc-8f77-ff1740244a44","price":174.32,"amount":3,"orderPriorEdit":{"orderId":"9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3","cliOrdId":"2gAFhnXarR0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":3,"filled":0,"limitPrice":174.40,"reduceOnly":false,"timestamp":"2025-05-28T12:51:43.750Z","lastUpdateTimestamp":"2025-05-28T12:53:51.897Z"},"orderPriorExecution":{"orderId":"9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3","cliOrdId":"2gAFhnXarR0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":3,"filled":0,"limitPrice":174.30,"reduceOnly":false,"timestamp":"2025-05-28T12:51:43.750Z","lastUpdateTimestamp":"2025-05-28T12:56:25.105Z"},"takerReducedQuantity":null,"type":"EXECUTION"}]}}"
W0528 14:56:25.126097 1140091 L0 order_entry.cpp:371] DEBUG edit_order={result=SUCCESS, error="", edit_status={order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3", cli_ord_id="", status=FILLED, received_time=1748436985106ms, order_events=[{order={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, old={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, new_={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, reduced_quantity=nan, type=EXECUTION, uid="", reason="", execution_id="5c58ccf8-22f5-4dbc-8f77-ff1740244a44", amount=3, price=174.32, order_prior_execution={order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3", cli_ord_id="2gAFhnXarR0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=3, filled=0, limit_price=174.3, reduce_only=false, timestamp=1748436703750ms, last_update_timestamp=1748436985105ms}, order_prior_edit={order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3", cli_ord_id="2gAFhnXarR0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=3, filled=0, limit_price=174.4, reduce_only=false, timestamp=1748436703750ms, last_update_timestamp=1748436831897ms}, taker_reduced_quantity=nan}]}, server_time=1748436985106ms}
W0528 14:56:25.126178 1140091 L0 order_entry.cpp:698] Exception type=N3roq12RuntimeErrorE, what="Unexpected: status=FILLED"
W0528 14:56:25.126201 1140091 L0 request_state.cpp:185] [OMS:XXX] Unexpected: had request_status=ACCEPTED, got request_status=ERROR


W0529 06:20:17.948902 1272728 L0 order_entry.cpp:373] DEBUG body="{"result":"success","serverTime":"2025-05-29T04:20:17.922Z","editStatus":{"status":"filled","orderId":"9f0642ca-ef01-4656-94d0-e715e0d90d4e","receivedTime":"2025-05-29T04:20:17.922Z","orderEvents":[{"executionId":"66804682-edc7-4ac0-a47c-d7f0afd06992","price":173.13,"amount":1.85,"orderPriorEdit":{"orderId":"9f0642ca-ef01-4656-94d0-e715e0d90d4e","cliOrdId":"pgAFKOP9zh0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":4,"filled":0,"limitPrice":173.50,"reduceOnly":false,"timestamp":"2025-05-29T04:19:52.158Z","lastUpdateTimestamp":"2025-05-29T04:19:52.158Z"},"orderPriorExecution":{"orderId":"9f0642ca-ef01-4656-94d0-e715e0d90d4e","cliOrdId":"pgAFKOP9zh0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":4,"filled":0,"limitPrice":173.10,"reduceOnly":false,"timestamp":"2025-05-29T04:19:52.158Z","lastUpdateTimestamp":"2025-05-29T04:20:17.922Z"},"takerReducedQuantity":null,"type":"EXECUTION"},{"executionId":"3793e8bf-bf8d-4bef-a024-5bfc0b7bdd01","price":173.12,"amount":2.15,"orderPriorEdit":{"orderId":"9f0642ca-ef01-4656-94d0-e715e0d90d4e","cliOrdId":"pgAFKOP9zh0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":4,"filled":0,"limitPrice":173.50,"reduceOnly":false,"timestamp":"2025-05-29T04:19:52.158Z","lastUpdateTimestamp":"2025-05-29T04:19:52.158Z"},"orderPriorExecution":{"orderId":"9f0642ca-ef01-4656-94d0-e715e0d90d4e","cliOrdId":"pgAFKOP9zh0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":2.15,"filled":1.85,"limitPrice":173.10,"reduceOnly":false,"timestamp":"2025-05-29T04:19:52.158Z","lastUpdateTimestamp":"2025-05-29T04:20:17.922Z"},"takerReducedQuantity":null,"type":"EXECUTION"}]}}"
W0529 06:20:17.949048 1272728 L0 order_entry.cpp:375] DEBUG edit_order={result=SUCCESS, error="", edit_status={order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e", cli_ord_id="", status=FILLED, received_time=1748492417922ms, order_events=[{order={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, old={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, new_={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, reduced_quantity=nan, type=EXECUTION, uid="", reason="", execution_id="66804682-edc7-4ac0-a47c-d7f0afd06992", amount=1.85, price=173.13, order_prior_execution={order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e", cli_ord_id="pgAFKOP9zh0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=4, filled=0, limit_price=173.1, reduce_only=false, timestamp=1748492392158ms, last_update_timestamp=1748492417922ms}, order_prior_edit={order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e", cli_ord_id="pgAFKOP9zh0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=4, filled=0, limit_price=173.5, reduce_only=false, timestamp=1748492392158ms, last_update_timestamp=1748492392158ms}, taker_reduced_quantity=nan}, {order={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, old={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, new_={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, reduced_quantity=nan, type=EXECUTION, uid="", reason="", execution_id="3793e8bf-bf8d-4bef-a024-5bfc0b7bdd01", amount=2.15, price=173.12, order_prior_execution={order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e", cli_ord_id="pgAFKOP9zh0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=2.15, filled=1.85, limit_price=173.1, reduce_only=false, timestamp=1748492392158ms, last_update_timestamp=1748492417922ms}, order_prior_edit={order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e", cli_ord_id="pgAFKOP9zh0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=4, filled=0, limit_price=173.5, reduce_only=false, timestamp=1748492392158ms, last_update_timestamp=1748492392158ms}, taker_reduced_quantity=nan}]}, server_time=1748492417922ms}
W0529 06:20:17.949138 1272728 L0 order_entry.cpp:719] Exception type=N3roq12RuntimeErrorE, what="Unexpected: size=2"
W0529 06:20:17.949149 1272728 L0 order_entry.cpp:426] DEBUG response={request_type=MODIFY_ORDER, origin=EXCHANGE, request_status=ERROR, error=UNKNOWN, text="Unexpected: size=2", version=2, request_id="", quantity=nan, price=nan}
W0529 06:20:17.949169 1272728 L0 request_state.cpp:185] [OMS:XXX] Unexpected: had request_status=ACCEPTED, got request_status=ERROR










W0529 09:25:10.334297 1305622 L0 drop_copy.cpp:376] DEBUG open_orders={feed=OPEN_ORDERS, order_id="9f068508-7244-4201-a7c1-2e57d2bd3ea7", cli_ord_id="JwAF6-us1R0AAQAAAAAA", order={instrument="", time=0ms, last_update_time=0ms, qty=nan, filled=nan, limit_price=nan, stop_price=nan, type=UNDEFINED_INTERNAL, order_id="", cli_ord_id="", direction=-1, reduce_only=false}, is_cancel=true, reason=CANCELLED_BY_USER}
W0529 09:25:10.341848 1305622 L0 order_entry.cpp:508] DEBUG body="{"result":"success","cancelStatus":{"status":"cancelled","receivedTime":"2025-05-29T07:25:10.317Z","orderEvents":[{"uid":"9f068508-7244-4201-a7c1-2e57d2bd3ea7","order":{"orderId":"9f068508-7244-4201-a7c1-2e57d2bd3ea7","cliOrdId":"JwAF6-us1R0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":1,"filled":0,"limitPrice":173.50,"reduceOnly":false,"timestamp":"2025-05-29T07:25:05.434Z","lastUpdateTimestamp":"2025-05-29T07:25:05.434Z"},"type":"CANCEL"}],"order_id":"9f068508-7244-4201-a7c1-2e57d2bd3ea7"},"serverTime":"2025-05-29T07:25:10.317Z"}"
W0529 09:25:10.341985 1305622 L0 order_entry.cpp:510] DEBUG cancel_order={result=SUCCESS, error="", cancel_status={order_id="9f068508-7244-4201-a7c1-2e57d2bd3ea7", cli_ord_id="", status=CANCELLED, received_time=1748503510317ms, order_events=[{order={order_id="9f068508-7244-4201-a7c1-2e57d2bd3ea7", cli_ord_id="JwAF6-us1R0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=1, filled=0, limit_price=173.5, reduce_only=false, timestamp=1748503505434ms, last_update_timestamp=1748503505434ms}, old={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, new_={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, reduced_quantity=nan, type=CANCEL, uid="9f068508-7244-4201-a7c1-2e57d2bd3ea7", reason="", execution_id="", amount=nan, price=nan, order_prior_execution={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, order_prior_edit={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, taker_reduced_quantity=nan}]}, server_time=1748503510317ms}
W0529 09:25:10.342036 1305622 L0 order_entry.cpp:539] DEBUG order_update={account="A1", exchange="", symbol="PF_SOLUSD", side=SELL, position_effect=UNDEFINED, margin_mode=UNDEFINED, max_show_quantity=nan, order_type=LIMIT, time_in_force=UNDEFINED, execution_instructions=, create_time_utc=0ns, update_time_utc=1748503510317000000ns, external_account="", external_order_id="9f068508-7244-4201-a7c1-2e57d2bd3ea7", client_order_id="", order_status=CANCELED, quantity=1, price=173.5, stop_price=nan, remaining_quantity=1, traded_quantity=0, average_traded_price=nan, last_traded_quantity=nan, last_traded_price=nan, last_liquidity=UNDEFINED, routing_id="", max_request_version=0, max_response_version=0, max_accepted_version=0, update_type=UNDEFINED, sending_time_utc=0ns}
W0529 09:25:10.342060 1305622 L0 order_entry.cpp:551] DEBUG response={request_type=CANCEL_ORDER, origin=EXCHANGE, request_status=ACCEPTED, error=UNDEFINED, text="", version=2, request_id="", quantity=nan, price=nan}
D0529 09:25:20.377963 1305622 L0 utils.cpp:165] DEBUG: quantity=1, precision={increment=1, precision=_0}, value=1, step=1, residual=0
D0529 09:25:20.377990 1305622 L0 utils.cpp:196] DEBUG: price=172.8, precision={increment=0.010000000000000002, precision=_2}, value=17280, step=1, residual=0
W0529 09:25:20.434462 1305622 L0 drop_copy.cpp:432] DEBUG fills={feed=FILLS, username="cc34f46b-bc3b-47a2-a7fa-e19e34e376f0", fills=[{instrument="PF_SOLUSD", time=1748503520415ms, price=172.81, seq=53, buy=false, qty=1, order_id="9f06851f-4e63-48fa-ad89-f7ac4598c70a", cli_ord_id="IAAF7Ous1R0AAQAAAAAA", fill_id="f640f041-53d3-4e60-a233-9072ce069498", fill_type=TAKER, fee_paid=0.08640500000000002, fee_currency="USD", taker_order_type="lmt", remaining_order_qty=0, order_type="lmt"}]}
W0529 09:25:20.436885 1305622 L0 order_entry.cpp:262] DEBUG body="{"result":"success","sendStatus":{"cliOrdId":"IAAF7Ous1R0AAQAAAAAA","status":"placed","receivedTime":"2025-05-29T07:25:20.415714Z","orderEvents":[{"executionId":"f640f041-53d3-4e60-a233-9072ce069498","price":172.8100000000,"amount":1,"orderPriorEdit":null,"orderPriorExecution":{"orderId":"9f06851f-4e63-48fa-ad89-f7ac4598c70a","cliOrdId":"IAAF7Ous1R0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":1,"filled":0,"limitPrice":172.80,"reduceOnly":false,"timestamp":"2025-05-29T07:25:20.415Z","lastUpdateTimestamp":"2025-05-29T07:25:20.415Z"},"takerReducedQuantity":null,"type":"EXECUTION"}],"order_id":"9f06851f-4e63-48fa-ad89-f7ac4598c70a"},"serverTime":"2025-05-29T07:25:20.416Z"}"
W0529 09:25:20.437035 1305622 L0 order_entry.cpp:264] DEBUG send_order={result=SUCCESS, error="", send_status={order_id="9f06851f-4e63-48fa-ad89-f7ac4598c70a", cli_ord_id="IAAF7Ous1R0AAQAAAAAA", status=PLACED, received_time=1748503520415ms, order_events=[{order={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, old={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, new_={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, reduced_quantity=nan, type=EXECUTION, uid="", reason="", execution_id="f640f041-53d3-4e60-a233-9072ce069498", amount=1, price=172.81, order_prior_execution={order_id="9f06851f-4e63-48fa-ad89-f7ac4598c70a", cli_ord_id="IAAF7Ous1R0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=1, filled=0, limit_price=172.8, reduce_only=false, timestamp=1748503520415ms, last_update_timestamp=1748503520415ms}, order_prior_edit={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, taker_reduced_quantity=nan}]}, server_time=1748503520416ms}
W0529 09:25:20.437116 1305622 L0 order_entry.cpp:293] DEBUG order_update={account="A1", exchange="", symbol="PF_SOLUSD", side=SELL, position_effect=UNDEFINED, margin_mode=UNDEFINED, max_show_quantity=nan, order_type=LIMIT, time_in_force=UNDEFINED, execution_instructions=, create_time_utc=0ns, update_time_utc=1748503520415000000ns, external_account="", external_order_id="9f06851f-4e63-48fa-ad89-f7ac4598c70a", client_order_id="", order_status=WORKING, quantity=nan, price=172.8, stop_price=nan, remaining_quantity=0, traded_quantity=1, average_traded_price=nan, last_traded_quantity=1, last_traded_price=172.81, last_liquidity=TAKER, routing_id="", max_request_version=0, max_response_version=0, max_accepted_version=0, update_type=UNDEFINED, sending_time_utc=0ns}
W0529 09:25:20.437140 1305622 L0 order_entry.cpp:305] DEBUG response={request_type=CREATE_ORDER, origin=EXCHANGE, request_status=ACCEPTED, error=UNDEFINED, text="", version=1, request_id="IAAF7Ous1R0AAQAAAAAA", quantity=nan, price=nan}
W0529 09:25:26.035071 1305622 L0 order_entry.cpp:508] DEBUG body="{"result":"success","cancelStatus":{"status":"notFound","receivedTime":"2025-05-29T07:25:26.014Z"},"serverTime":"2025-05-29T07:25:26.014Z"}"
W0529 09:25:26.035131 1305622 L0 order_entry.cpp:510] DEBUG cancel_order={result=SUCCESS, error="", cancel_status={order_id="", cli_ord_id="", status=NOT_FOUND, received_time=1748503526014ms, order_events=[]}, server_time=1748503526014ms}
W0529 09:25:26.035154 1305622 L0 order_entry.cpp:567] DEBUG response={request_type=CANCEL_ORDER, origin=EXCHANGE, request_status=REJECTED, error=UNKNOWN, text="notFound", version=2, request_id="", quantity=nan, price=nan}



D0529 10:56:47.562761 1316014 L0 utils.cpp:165] DEBUG: quantity=1, precision={increment=1, precision=_0}, value=1, step=1, residual=0
D0529 10:56:47.562782 1316014 L0 utils.cpp:196] DEBUG: price=173, precision={increment=0.010000000000000002, precision=_2}, value=17300, step=1, residual=0
W0529 10:56:47.611842 1316014 L0 drop_copy.cpp:432] DEBUG fills={feed=FILLS, username="cc34f46b-bc3b-47a2-a7fa-e19e34e376f0", fills=[{instrument="PF_SOLUSD", time=1748509007590ms, price=173.01, seq=64, buy=false, qty=1, order_id="9f06a59c-e72c-46da-99da-4002fb8d3706", cli_ord_id="xQAFbfXb2B0AAQAAAAAA", fill_id="75859b03-9740-4c41-9c34-17d956a9c2f1", fill_type=TAKER, fee_paid=0.08650500000000003, fee_currency="USD", taker_order_type="lmt", remaining_order_qty=0, order_type="lmt"}]}
W0529 10:56:47.611997 1316014 L0 drop_copy.cpp:376] DEBUG open_orders={feed=OPEN_ORDERS, order_id="", cli_ord_id="", order={instrument="PF_SOLUSD", time=1748508971435ms, last_update_time=1748509007590ms, qty=0, filled=1, limit_price=173, stop_price=0, type=LIMIT, order_id="9f06a59c-e72c-46da-99da-4002fb8d3706", cli_ord_id="xQAFbfXb2B0AAQAAAAAA", direction=1, reduce_only=false}, is_cancel=true, reason=FULL_FILL}
W0529 10:56:47.613528 1316014 L0 order_entry.cpp:385] DEBUG body="{"result":"success","serverTime":"2025-05-29T08:56:47.591Z","editStatus":{"status":"filled","orderId":"9f06a59c-e72c-46da-99da-4002fb8d3706","receivedTime":"2025-05-29T08:56:47.591Z","orderEvents":[{"executionId":"75859b03-9740-4c41-9c34-17d956a9c2f1","price":173.01,"amount":1,"orderPriorEdit":{"orderId":"9f06a59c-e72c-46da-99da-4002fb8d3706","cliOrdId":"xQAFbfXb2B0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":3,"filled":0,"limitPrice":174.00,"reduceOnly":false,"timestamp":"2025-05-29T08:56:11.435Z","lastUpdateTimestamp":"2025-05-29T08:56:11.435Z"},"orderPriorExecution":{"orderId":"9f06a59c-e72c-46da-99da-4002fb8d3706","cliOrdId":"xQAFbfXb2B0AAQAAAAAA","type":"lmt","symbol":"PF_SOLUSD","side":"sell","quantity":1,"filled":0,"limitPrice":173.00,"reduceOnly":false,"timestamp":"2025-05-29T08:56:11.435Z","lastUpdateTimestamp":"2025-05-29T08:56:47.590Z"},"takerReducedQuantity":null,"type":"EXECUTION"}]}}"
W0529 10:56:47.613626 1316014 L0 order_entry.cpp:387] DEBUG edit_order={result=SUCCESS, error="", edit_status={order_id="9f06a59c-e72c-46da-99da-4002fb8d3706", cli_ord_id="", status=FILLED, received_time=1748509007591ms, order_events=[{order={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, old={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, new_={order_id="", cli_ord_id="", type=UNDEFINED_INTERNAL, symbol="", side=UNDEFINED_INTERNAL, quantity=nan, filled=nan, limit_price=nan, reduce_only=false, timestamp=0ms, last_update_timestamp=0ms}, reduced_quantity=nan, type=EXECUTION, uid="", reason="", execution_id="75859b03-9740-4c41-9c34-17d956a9c2f1", amount=1, price=173.01, order_prior_execution={order_id="9f06a59c-e72c-46da-99da-4002fb8d3706", cli_ord_id="xQAFbfXb2B0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=1, filled=0, limit_price=173, reduce_only=false, timestamp=1748508971435ms, last_update_timestamp=1748509007590ms}, order_prior_edit={order_id="9f06a59c-e72c-46da-99da-4002fb8d3706", cli_ord_id="xQAFbfXb2B0AAQAAAAAA", type=LMT, symbol="PF_SOLUSD", side=SELL, quantity=3, filled=0, limit_price=174, reduce_only=false, timestamp=1748508971435ms, last_update_timestamp=1748508971435ms}, taker_reduced_quantity=nan}]}, server_time=1748509007591ms}
W0529 10:56:47.613666 1316014 L0 order_entry.cpp:416] DEBUG order_update={account="A1", exchange="", symbol="PF_SOLUSD", side=SELL, position_effect=UNDEFINED, margin_mode=UNDEFINED, max_show_quantity=nan, order_type=LIMIT, time_in_force=UNDEFINED, execution_instructions=, create_time_utc=0ns, update_time_utc=1748509007591000000ns, external_account="", external_order_id="9f06a59c-e72c-46da-99da-4002fb8d3706", client_order_id="", order_status=COMPLETED, quantity=nan, price=173, stop_price=nan, remaining_quantity=0, traded_quantity=1, average_traded_price=nan, last_traded_quantity=1, last_traded_price=173.01, last_liquidity=TAKER, routing_id="", max_request_version=0, max_response_version=0, max_accepted_version=0, update_type=UNDEFINED, sending_time_utc=0ns}
W0529 10:56:47.613681 1316014 L0 order_entry.cpp:428] DEBUG response={request_type=MODIFY_ORDER, origin=EXCHANGE, request_status=ACCEPTED, error=UNDEFINED, text="", version=2, request_id="", quantity=nan, price=nan}
W0529 10:56:47.613714 1316014 L0 order_state.cpp:1052] [OMS:128192345453] *** CHANGE(TRADED QUANTITY 1 ==> 1) (0) != LAST TRADED QUANTITY (1) ***
W0529 10:56:47.613720 1316014 L0 order_state.cpp:1059] [OMS:128192345453] *** RESET LAST TRADED ***




# Send Order

## Maker

```
type=PLACED
status=PLACE
```

```
send_order={
    result=SUCCESS,
    error="",
    send_status={
    order_id="9f061a7e-8ae6-45bc-a8eb-15aee1fa3afa",
    cli_ord_id="AgAFkwEFyx0AAQAAAAAA",
X   status=PLACED,
    received_time=1748485631208ms,
    order_events=[{
        order={
            order_id="9f061a7e-8ae6-45bc-a8eb-15aee1fa3afa",
            cli_ord_id="AgAFkwEFyx0AAQAAAAAA",
            type=LMT,
            symbol="PF_SOLUSD",
            side=BUY,
X           quantity=1,
X           filled=0,
X           limit_price=150,
            reduce_only=false,
            timestamp=1748485631208ms,
            last_update_timestamp=1748485631208ms
        },
        old={
            order_id="",
            cli_ord_id="",
            type=UNDEFINED_INTERNAL,
            symbol="",
            side=UNDEFINED_INTERNAL,
            quantity=nan,
            filled=nan,
            limit_price=nan,
            reduce_only=false,
            timestamp=0ms,
            last_update_timestamp=0ms
        },
        new_={
            order_id="",
            cli_ord_id="",
            type=UNDEFINED_INTERNAL,
            symbol="",
            side=UNDEFINED_INTERNAL,
            quantity=nan,
            filled=nan,
            limit_price=nan,
            reduce_only=false,
            timestamp=0ms,
            last_update_timestamp=0ms
        },
?       reduced_quantity=nan,
X       type=PLACE,
        uid="",
        reason="",
        execution_id="",
        amount=nan,
        price=nan,
        order_prior_execution={
            order_id="",
            cli_ord_id="",
            type=UNDEFINED_INTERNAL,
            symbol="",
            side=UNDEFINED_INTERNAL,
            quantity=nan,
            filled=nan,
            limit_price=nan,
            reduce_only=false,
            timestamp=0ms,
            last_update_timestamp=0ms
        },
        order_prior_edit={
            order_id="",
            cli_ord_id="",
            type=UNDEFINED_INTERNAL,
            symbol="",
            side=UNDEFINED_INTERNAL,
            quantity=nan,
            filled=nan,
            limit_price=nan,
            reduce_only=false,
            timestamp=0ms,
            last_update_timestamp=0ms
        },
        taker_reduced_quantity=nan
        }
    ]},
    server_time=1748485631209ms
}
```

## Taker

```
status=PLACED
type=EXECUTION
```

```
send_order={
    result=SUCCESS,
    error="",
    send_status={
        order_id="9f0615f7-4e7c-4348-a8ac-f62dfe3d848d",
        cli_ord_id="zwAFoWuRyh0AAQAAAAAA",
X       status=PLACED,
        received_time=1748484871491ms,
        order_events=[{
            order={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            old={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            new_={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
?           reduced_quantity=nan,
X           type=EXECUTION,
            uid="",
            reason="",
            execution_id="eb67576c-875b-42de-a4c4-dbf18c5428e3",
X           amount=1,
X           price=172.36,
            order_prior_execution={
                order_id="9f0615f7-4e7c-4348-a8ac-f62dfe3d848d",
                cli_ord_id="zwAFoWuRyh0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=BUY,
X               quantity=1,
X               filled=0,
X               limit_price=174,
                reduce_only=false,
                timestamp=1748484871491ms,
                last_update_timestamp=1748484871491ms
            },
            order_prior_edit={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
?           taker_reduced_quantity=nan
        }]
    },
    server_time=1748484871492ms
}
```


# Edit Order

## Maker

```
status=EDITED
type=EDIT
```

```
edit_order={
    result=SUCCESS,
    error="",
    edit_status={
        order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3",
        cli_ord_id="",
        status=EDITED,
        received_time=1748436831898ms,
        order_events=[{
            order={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            old={
                order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3",
                cli_ord_id="2gAFhnXarR0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=SELL,
                quantity=3,
                filled=0,
                limit_price=174.5,
                reduce_only=false,
                timestamp=1748436703750ms,
                last_update_timestamp=1748436731745ms
            },
            new_={
                order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3",
                cli_ord_id="2gAFhnXarR0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
X               side=SELL,
X               quantity=3,
X               filled=0,
X               limit_price=174.4,
                reduce_only=false,
                timestamp=1748436703750ms,
                last_update_timestamp=1748436831897ms
            },
            reduced_quantity=nan,
            type=EDIT,
            uid="",
            reason="",
            execution_id="",
            amount=nan,
            price=nan,
            order_prior_execution={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            order_prior_edit={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            taker_reduced_quantity=nan
        }]
    },
    server_time=1748436831898ms
}
```

## Taker

```
status=FILLED
type=EXECUTION
```

```
edit_order={
    result=SUCCESS,
    error="",
    edit_status={
        order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3",
        cli_ord_id="",
        status=FILLED,
        received_time=1748436985106ms,
        order_events=[{
            order={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            old={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            new_={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            reduced_quantity=nan,
            type=EXECUTION,
            uid="",
            reason="",
            execution_id="5c58ccf8-22f5-4dbc-8f77-ff1740244a44",
X           amount=3,
X           price=174.32,
            order_prior_execution={
                order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3",
                cli_ord_id="2gAFhnXarR0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=SELL,
X               quantity=3,
X               filled=0,
X               limit_price=174.3,
                reduce_only=false,
                timestamp=1748436703750ms,
                last_update_timestamp=1748436985105ms
            },
            order_prior_edit={
                order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3",
                cli_ord_id="2gAFhnXarR0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=SELL,
                quantity=3,
                filled=0,
                limit_price=174.4,
                reduce_only=false,
                timestamp=1748436703750ms,
                last_update_timestamp=1748436831897ms
            },
            taker_reduced_quantity=nan
        }]
    },
    server_time=1748436985106ms
}
```

## Rejected

```
status=ORDER_FOR_EDIT_NOT_FOUND
```

```
edit_order={
    result=SUCCESS,
    error="",
    edit_status={
        order_id="9f0626da-3d36-4284-b3e4-9983bfc9b2dd",
        cli_ord_id="",
        status=ORDER_FOR_EDIT_NOT_FOUND,
        received_time=1748487730803ms,
        order_events=[]
    },
    server_time=1748487730803ms
}
```

## Taker (multiple fills)

```
status=FILLED
type=EXECUTION
```

```
edit_order={
    result=SUCCESS,
    error="",
    edit_status={
        order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e",
        cli_ord_id="",
        status=FILLED,
        received_time=1748492417922ms,
        order_events=[{
            order={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            old={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            new_={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            reduced_quantity=nan,
            type=EXECUTION,
            uid="",
            reason="",
            execution_id="66804682-edc7-4ac0-a47c-d7f0afd06992",
X           amount=1.85,
X           price=173.13,
            order_prior_execution={
                order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e",
                cli_ord_id="pgAFKOP9zh0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=SELL,
X               quantity=4,
X               filled=0,
                limit_price=173.1,
                reduce_only=false,
                timestamp=1748492392158ms,
                last_update_timestamp=1748492417922ms
            },
            order_prior_edit={
                order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e",
                cli_ord_id="pgAFKOP9zh0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=SELL,
                quantity=4,
                filled=0,
                limit_price=173.5,
                reduce_only=false,
                timestamp=1748492392158ms,
                last_update_timestamp=1748492392158ms
            },
            taker_reduced_quantity=nan
        }, {
            order={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            old={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            new_={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            reduced_quantity=nan,
            type=EXECUTION,
            uid="",
            reason="",
            execution_id="3793e8bf-bf8d-4bef-a024-5bfc0b7bdd01",
X           amount=2.15,
X           price=173.12,
            order_prior_execution={
                order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e",
                cli_ord_id="pgAFKOP9zh0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=SELL,
X               quantity=2.15,
X               filled=1.85,
X               limit_price=173.1,
                reduce_only=false,
                timestamp=1748492392158ms,
                last_update_timestamp=1748492417922ms
            },
            order_prior_edit={
                order_id="9f0642ca-ef01-4656-94d0-e715e0d90d4e",
                cli_ord_id="pgAFKOP9zh0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=SELL,
                quantity=4,
                filled=0,
                limit_price=173.5,
                reduce_only=false,
                timestamp=1748492392158ms,
                last_update_timestamp=1748492392158ms
            },
            taker_reduced_quantity=nan
        }]
    },
    server_time=1748492417922ms
}
```

MISSING: partially filled

# Cancel

## Simple

```
status=CANCELLED
type=CANCEL
```

```
cancel_order={
    result=SUCCESS,
    error="",
    cancel_status={
        order_id="9f061a7e-8ae6-45bc-a8eb-15aee1fa3afa",
        cli_ord_id="",
        status=CANCELLED,
        received_time=1748485955111ms,
        order_events=[{
            order={
                order_id="9f061a7e-8ae6-45bc-a8eb-15aee1fa3afa",
                cli_ord_id="AgAFkwEFyx0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=BUY,
                quantity=1,
                filled=0,
                limit_price=150,
                reduce_only=false,
                timestamp=1748485631208ms,
                last_update_timestamp=1748485631208ms
            },
            old={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            new_={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            reduced_quantity=nan,
            type=CANCEL,
            uid="9f061a7e-8ae6-45bc-a8eb-15aee1fa3afa",
            reason="",
            execution_id="",
            amount=nan,
            price=nan,
            order_prior_execution={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            order_prior_edit={
                order_id="",
                cli_ord_id="",
                type=UNDEFINED_INTERNAL,
                symbol="",
                side=UNDEFINED_INTERNAL,
                quantity=nan,
                filled=nan,
                limit_price=nan,
                reduce_only=false,
                timestamp=0ms,
                last_update_timestamp=0ms
            },
            taker_reduced_quantity=nan
        }]
    },
    server_time=1748485955111ms
}
```

## Rejected

```
status=NOT_FOUND
```

```
cancel_order={
    result=SUCCESS,
    error="",
    cancel_status={
        order_id="",
        cli_ord_id="",
        status=NOT_FOUND,
        received_time=1748486382792ms,
        order_events=[]
    },
    server_time=1748486382792ms}
```
