.. _roq-kraken-futures:

.. |checkmark| unicode:: U+2713

roq-kraken-futures
==================


Links
-----

* `Website <https://futures.kraken.com/>`__
* `Demo <https://demo-futures.kraken.com/futures/PI_XBTUSD>`__
* `Support <https://support.kraken.com/hc/en-us/categories/360001977131-Futures>`__
* `API <https://support.kraken.com/hc/en-us/sections/360012894412-Futures-API>`__
* `Reddit Support <https://www.reddit.com/r/KrakenSupport/>`__
* `Telegram API <https://t.me/kraken_futures_api>`__
* `Telegram General <https://t.me/kraken_futures>`__


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto

      * - Spot
        -
      * - Futures
        - |checkmark|
      * - Options
        -
      * - Combos
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto

      * - Reference Data
        - |checkmark|
      * - Market Status
        - |checkmark|
      * - Top of Book
        - |checkmark|
      * - Market by Price
        - |checkmark|
      * - Market by Order
        -
      * - Trade Summary
        - |checkmark|
      * - Statistics
        - |checkmark|
      * - Time Series
        - |checkmark|

  .. grid-item-card::  Order Management

    .. list-table::
      :widths: auto

      * - Create
        - |checkmark|
      * - Modify
        - |checkmark|
      * - Cancel
        - |checkmark|
      * - Cancel All
        - |checkmark|
      * - Auto-Cancel
        - |checkmark|

  .. grid-item-card::  Account Management

    .. list-table::
      :widths: auto

      * - Positions
        - |checkmark|
      * - Funds
        - |checkmark|


Installing
----------

* :ref:`Using Conda <tutorial-conda>`

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


Using
-----

.. code-block:: shell

   $ roq-kraken-futures \
         --name "kraken-futures" \
         --config_file $CONFIG_FILE_PATH \
         --client_listen_address $UNIX_SOCKET_PATH \
         --flagfile $ENVIRONMENT_FLAGFILE


.. _roq-kraken-futures-flags:

Flags
-----

* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`

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

      $ $CONDA_PREFIX/share/roq-kraken-futures/flags/prod/flags.cfg

   .. include:: flags/prod/flags.cfg
     :code: shell

.. tab:: Test

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-kraken-futures/flags/test/flags.cfg

   .. include:: flags/test/flags.cfg
     :code: shell


Configuration
-------------

* :ref:`Gateway Config <gateway-config>`

.. code-block:: shell

   $ $CONDA_PREFIX/share/roq-kraken-futures/config.toml

.. important::

   The template will be replaced when the software is upgraded.
   Make a copy and modify to your needs.

.. include:: config.toml
   :code: toml


Market Data
-----------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      -
      -
      -

    * - :cpp:class:`roq::MarketStatus`
      - MarketData
      - ticker
      -

    * - :cpp:class:`roq::TopOfBook`
      - MarketData
      - ticker
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - MarketData
      - book
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      - Unavailable

    * - :cpp:class:`roq::TradeSummary`
      - MarketData
      - trade
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      - MarketData
      - ticker
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      - Rest
      - instruments
      -

    * - :cpp:class:`roq::MarketStatus`
      -
      -
      -

    * - :cpp:class:`roq::TopOfBook`
      -
      -
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      -
      -
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeSummary`
      -
      -
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      -
      -
      -

Statistics
~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`INDEX_VALUE`
    - (ticker) :code:`index`

  * - :cpp:class:`SETTLEMENT_PRICE`
    - (ticker) :code:`markPrice`

  * - :cpp:class:`FUNDING_RATE`
    - (ticker) :code:`relative_funding_rate`

  * - :cpp:class:`FUNDING_RATE_PREDICTION`
    - (ticker) :code:`relative_funding_rate_prediction`


Order Management
----------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - DropCopy
      - open_orders
      -

    * - :cpp:class:`roq::TradeUpdate`
      - DropCopy
      - fills
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - DropCopy
      - open_orders_snapshot
      -

    * - :cpp:class:`roq::TradeUpdate`
      - DropCopy
      - fills_snapshot
      -

.. tab:: Request

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::CreateOrder`
      - OrderEntry
      - /api/v3/sendorder
      -

    * - :cpp:class:`roq::ModifyOrder`
      - OrderEntry
      - /api/v3/editorder
      -

    * - :cpp:class:`roq::CancelOrder`
      - OrderEntry
      - /api/v3/cancelorder
      -

    * - :cpp:class:`roq::CancelAllOrders`
      - OrderEntry
      - /api/v3/canceallorders
      -

.. tab:: Response

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderAck`
      -
      -
      -

Order Types
~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`MARKET`
    - Mapped to :code:`mkt` (JSON)

  * - :cpp:class:`LIMIT`
    - Mapped to :code:`lmt` (JSON)


Time in Force
~~~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`GTC`
    - Mapped to :code:`ioc=false` (JSON)

  * - :cpp:class:`IOC`
    - Mapped to :code:`ioc=true` (JSON)


Position Effect
~~~~~~~~~~~~~~~

.. note::

  Not supported



Execution Instructions
~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`DO_NOT_INCREASE`
    - Mapped to :code:`reduce_only=true` (JSON)


Account Management
------------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      - DropCopy
      - open_positions
      -

    * - :cpp:class:`roq::FundsUpdate`
      - DropCopy
      - account_balances_and_margins
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      -
      -
      -

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      -


Streams
-------

.. tab:: Rest

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - REST
      - Primary purpose

        * Download instruments

.. tab:: MarketData

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose

        * live market data (top of book + market status)

        Each connection

        * supports a slice of the symbols

.. tab:: OrderEntry

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - REST
      - Primary purpose

        * support order management

        Each connection

        * supports a single account

.. tab:: DropCopy

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose

        * live account updates, including positions and funds

        Each connection

        * supports a single account


Constraints
-----------

* The field :code:`cliOrdId` is a string containing only web-safe characters.

Comments
--------

* Statistics are using relative funding rates to be compatible with other exchanges

* Aggressive orders may only report fills on the WS channel (an open order never exists).
  This is an issue because we can potentially lose the REST response and then never be able to update the order status.
  An artificial :code:`OrderUpdate` is therefore injected when receiving fills from the WS channel.

* Order updates have no information about last traded price/quantity and total average price.
  Although attempts are made to compute the right average price, this is not always possible and errors should be expected.

* Time-series can only be downloaded.
  There is no web-socket API for subscribing.
