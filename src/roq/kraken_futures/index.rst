.. _roq-kraken-futures:

.. |dagger| unicode:: U+2020
.. |double-dagger| unicode:: U+2021
.. |right-arrow| unicode:: U+2192
.. |right-double-arrow| unicode:: U+21D2
.. |left-right-double-arrow| unicode:: U+21D4
.. |check-mark| unicode:: U+2705
.. |cross-mark| unicode:: U+274C
.. |negative-cross-mark| unicode:: U+274E
.. |footnote-1| unicode:: U+2776
.. |footnote-2| unicode:: U+2777
.. |footnote-3| unicode:: U+2778


`roq-kraken-futures <https://github.com/roq-trading/roq-kraken-futures/>`__
===========================================================================

.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-kraken-futures

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-kraken-futures


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:enumerator:`Spot <roq::SecurityType::SPOT>`
        - |cross-mark|
        -
      * - :cpp:enumerator:`Futures <roq::SecurityType::FUTURES>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Swap <roq::SecurityType::SWAP>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Option <roq::SecurityType::OPTION>`
        - |cross-mark|
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`ReferenceData <roq::ReferenceData>`
        - |check-mark|
        -
      * - :cpp:class:`MarketStatus <roq::MarketStatus>`
        - |check-mark|
        -
      * - :cpp:class:`TopOfBook <roq::TopOfBook>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByPrice <roq::MarketByPriceUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByOrder <roq::MarketByOrderUpdate>`
        - |cross-mark|
        -
      * - :cpp:class:`TradeSummary <roq::TradeSummary>`
        - |check-mark|
        -
      * - :cpp:class:`Statistics <roq::StatisticsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`TimeSeries <roq::TimeSeriesUpdate>`
        - |check-mark|
        -

  .. grid-item-card::  Orders & Quotes

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`CreateOrder <roq::CreateOrder>`
        - |check-mark|
        -
      * - :cpp:class:`ModifyOrder <roq::ModifyOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelOrder <roq::CancelOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelAllOrders <roq::CancelAllOrders>`
        - |check-mark|
        -
      * - :cpp:class:`MassQuote <roq::MassQuote>`
        - |cross-mark|
        -
      * - :cpp:class:`CancelQuotes <roq::CancelQuotes>`
        - |cross-mark|
        -

  .. grid-item-card::  Account

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`Funds <roq::FundsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`Position <roq::PositionUpdate>`
        - |check-mark|
        -

.. note::

   |check-mark| = Available.

   |negative-cross-mark| = Not implemented.

   |cross-mark| = Unavailable.


Using
-----

.. code-block:: shell

   $ roq-kraken-futures [FLAGS]


.. _roq-kraken-futures-flags:

Flags
-----

.. code-block:: shell

   $ roq-kraken-futures --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc

.. tab:: Request

   .. include:: flags/request.rstinc


Environments
------------

.. tab:: Prod

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-kraken-futures/flags/prod/flags.cfg

   .. include:: flags/prod/flags.cfg
     :code: shell

.. tab:: Test

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-kraken-futures/flags/test/flags.cfg

   .. include:: flags/test/flags.cfg
     :code: shell


Configuration
-------------

.. code-block:: shell

   $ --config_file $CONDA_PREFIX/share/roq-kraken-futures/config.toml

.. important::

   This template will be replaced when the software is upgraded.
   Make a copy and modify to your own needs.

.. include:: config.toml
   :code: toml


Market Data
-----------


Inbound
~~~~~~~

.. tab:: TradingStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Event
       - Field
       - Value
       -
       -

     * - :code:`ticker`
       - :code:`suspended`
       - :code:`false`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN <roq::TradingStatus::OPEN>`

     * - :code:`ticker`
       - :code:`suspended`
       - :code:`true`
       - |right-double-arrow|
       - :cpp:enumerator:`CLOSE <roq::TradingStatus::CLOSE>`


.. tab:: StatisticsType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Event
       - Field
       -
       -

     * - :code:`ticker`
       - :code:`open`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN_PRICE <roq::StatisticsType::OPEN_PRICE>`

     * - :code:`ticker`
       - :code:`mark_price`
       - |right-double-arrow|
       - :cpp:enumerator:`SETTLEMENT_PRICE <roq::StatisticsType::SETTLEMENT_PRICE>`

     * - :code:`ticker`
       - :code:`open_interest`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN_INTEREST <roq::StatisticsType::OPEN_INTEREST>`

     * - :code:`ticker`
       - :code:`high`
       - |right-double-arrow|
       - :cpp:enumerator:`HIGHEST_TRADED_PRICE <roq::StatisticsType::HIGHEST_TRADED_PRICE>`

     * - :code:`ticker`
       - :code:`low`
       - |right-double-arrow|
       - :cpp:enumerator:`LOWEST_TRADED_PRICE <roq::StatisticsType::LOWEST_TRADED_PRICE>`

     * - :code:`ticker`
       - :code:`relative_funding_rate`
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE <roq::StatisticsType::FUNDING_RATE>`

     * - :code:`ticker`
       - :code:`relative_funding_rate_prediction`
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE_PREDICTION <roq::StatisticsType::FUNDING_RATE_PREDICTION>`

     * - :code:`ticker`
       - :code:`volume`
       - |right-double-arrow|
       - :cpp:enumerator:`TRADE_VOUME <roq::StatisticsType::TRADE_VOUME>`


Order Management
----------------


Inbound
~~~~~~~

.. tab:: OrderType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`mkt`
       - |right-double-arrow|
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`

     * - :code:`lmt`
       - |right-double-arrow|
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`


.. tab:: TimeInForce

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`GTC`
       - |right-double-arrow|
       - :cpp:enumerator:`GTC <roq::TimeInForce::GTC>`

     * - :code:`IOC`
       - |right-double-arrow|
       - :cpp:enumerator:`IOC <roq::TimeInForce::IOC>`


Outbound
~~~~~~~~

.. tab:: CreateOrder

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - :cpp:member:`order_type <roq::CreateOrder::order_type>`
       - :cpp:member:`execution_instructions <roq::CreateOrder::execution_instructions>`
       - :cpp:member:`price <roq::CreateOrder::price>`
       - :cpp:member:`stop_price <roq::CreateOrder::stop_price>`
       -
       - :code:`orderType`
       - :code:`limitPrice`
       - :code:`stopPrice`
       - :code:`reduceOnly`

     * - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`mkt`
       - |cross-mark|
       - |cross-mark|
       -

     * - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - |check-mark|
       - |right-double-arrow|
       - :code:`mkt`
       - |cross-mark|
       - |check-mark|
       -

     * - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`LIMIT`
       - |check-mark|
       - |cross-mark|
       -

     * - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - |check-mark|
       - |right-double-arrow|
       - :code:`mkt`
       - |check-mark|
       - |check-mark|
       -


.. tab:: ModifyOrder

   TBD


.. tab:: CancelOrder

   TBD


.. tab:: CancelAllOrders

   TBD



Comments
--------

* The field :code:`cliOrdId` is a string containing only web-safe characters.

* Statistics are using relative funding rates to be compatible with other exchanges

* Aggressive orders may only report fills on the WS channel (an open order never exists).
  This is an issue because we can potentially lose the REST response and then never be able to update the order status.
  An artificial :code:`OrderUpdate` is therefore injected when receiving fills from the WS channel.

* Order updates have no information about last traded price/quantity and total average price.
  Although attempts are made to compute the right average price, this is not always possible and errors should be expected.

* Time-series can only be downloaded.
  There is no web-socket API for subscribing.


References
----------


Common
~~~~~~

* :ref:`Using Conda <tutorial-conda>`
* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`
* :ref:`Gateway Config <gateway-config>`


GitHub
~~~~~~

* `roq-kraken-futures <https://github.com/roq-trading/roq-kraken-futures/>`__


Exchange
~~~~~~~~

* `Website <https://www.kraken.com/features/futures/>`__
* `Documentation <https://docs.kraken.com/api/>`__
