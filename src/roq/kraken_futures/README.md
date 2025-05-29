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
    status=PLACED,
    received_time=1748485631208ms,
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
        type=PLACE,
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
        status=PLACED,
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
            reduced_quantity=nan,
            type=EXECUTION,
            uid="",
            reason="",
            execution_id="eb67576c-875b-42de-a4c4-dbf18c5428e3",
            amount=1,
            price=172.36,
            order_prior_execution={
                order_id="9f0615f7-4e7c-4348-a8ac-f62dfe3d848d",
                cli_ord_id="zwAFoWuRyh0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=BUY,
                quantity=1,
                filled=0,
                limit_price=174,
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
            taker_reduced_quantity=nan
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
                side=SELL,
                quantity=3,
                filled=0,
                limit_price=174.4,
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
            amount=3,
            price=174.32,
            order_prior_execution={
                order_id="9f04f6dd-2a83-4b9b-aabf-0be277a9d0a3",
                cli_ord_id="2gAFhnXarR0AAQAAAAAA",
                type=LMT,
                symbol="PF_SOLUSD",
                side=SELL,
                quantity=3,
                filled=0,
                limit_price=174.3,
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
