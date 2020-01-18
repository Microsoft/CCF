// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#include "perf_client.h"
#include <ctime>

using namespace std;
using namespace nlohmann;

using Base = client::PerfBase;
using ConnPtr = std::shared_ptr<RpcTlsClient>;

class TpccClient : public Base
{
private:
  enum class TransactionTypes : uint8_t
  {
    NewOrder = 0,

    NumberTransactions
  };

  uint64_t num_warehouses = 5;

  void send_creation_transactions(const ConnPtr& connection) override
  {
    static const uint64_t num_districts = 10;
    static const uint64_t num_customers = 3000;
    static const uint64_t num_orders = 3000;
    static const uint64_t num_new_orders = 900;

    // Load the Items table
    load_items(connection);

    // Load the Warehouses, for each warehouse, load districts and stocks
    for (uint64_t w_id = 1; w_id <= num_warehouses; w_id++)
    {
      load_warehouse(connection, w_id);
      load_stocks(connection, w_id);

      // Load districts, for each district, load customers and orders
      for (uint64_t d_id = 1; d_id <= num_districts; d_id++)
      {
        load_district(connection, d_id, w_id);

        // Find customer IDs with bad credit (10%)
        std::unordered_set<uint64_t> bad_credit_ids = select_n_unique(num_customers / 10, 1, num_customers);

        // Load customers. For each customer, load history
        for (uint64_t c_id = 1; c_id <= num_customers; c_id++)
        {
          bool bad_credit = bad_credit_ids.find(c_id) != bad_credit_ids.end();

          load_customer(connection, c_id, d_id, w_id, bad_credit);
          load_history(connection, c_id, d_id, w_id);
        }

        // Create random permutation for customer IDs
        int* c_id_perms = permutation(1, num_orders);

        // Load orders. For each order, load order line
        for (uint64_t o_id = 1; o_id <= num_orders; o_id++)
        {
          uint64_t o_ol_cnt = rand_range(5, 15);
          load_order(connection, o_id, o_ol_cnt, d_id, w_id, c_id_perms[o_id]);
          load_order_lines(connection, o_id, o_ol_cnt, d_id, w_id);
        }

        // Load new orders for the last 900 order Ids
        load_new_orders(connection, num_orders - num_new_orders, num_orders, d_id, w_id);
      }
    }
  }

  void prepare_transactions() override
  {
    // Reserve space for transactions
    prepared_txs.resize(num_transactions);

    for (decltype(num_transactions) i = 0; i < num_transactions; i++)
    {
      // For now we just do 'NewOrder' transactions, in future we should
      // select each transaction using random sampling.

      json params = generate_new_order_params();

      add_prepared_tx("TPCC_new_order", params, true, i);
    }

  }

  json generate_new_order_params()
  {
    // TODO: refactor constants

    json params;

    // Warehouse ID
    uint64_t w_id = rand_range(num_warehouses) + 1;
    params["w_id"] = w_id;

    // District ID: Rand[1, 10] from home warehouse
    params["d_id"] = rand_range(1, 11);

    // Customer ID: NURand[1023, 1, 3000] from district number
    params["c_id"] = nu_rand(1023, 1, 3000);

    // Entry Date: current date time
    std::time_t t = std::time(0);
    params["o_entry_d"] = ctime(&t);

    // Number of items: Rand[5, 15]
    uint64_t ol_cnt = rand_range(5, 16);
    
    params["i_ids"] = {};
    params["i_w_ids"] = {};
    params["i_qtys"] = {};

    // 1% of transactions will rollback
    bool rollback = rand_range(0, 100) == 0;

    // Generate Items
    for (size_t i = 1; i <= ol_cnt; i++)
    {
      // Item Id: NURand[8191, 1, 100000]
      uint64_t i_id = nu_rand(8191, 1, 100000);

      if (rollback && i == ol_cnt)
      {
        i_id = 100001; // Unused value
      }

      params["i_ids"].push_back(i_id);

      // Supplying Warehouse: 99% home, 1% remote
      uint64_t o_supply_w_id = w_id;
      if (rand_range(0, 100) == 0)
      {
        do
        {
          o_supply_w_id = rand_range(num_warehouses) + 1;
        }
        while (o_supply_w_id == w_id);
      }

      params["i_w_ids"].push_back(o_supply_w_id);

      // Quantity: Rand[1, 10]
      params["i_qtys"].push_back(rand_range(1, 11));
    }

    return params;
  }

  /* ----- Data loading methods ----- */

  void load_items(const ConnPtr& connection)
  {
    static uint64_t num_items = 100000; // TODO: refactor constants
    std::unordered_set<uint64_t> original_rows = select_n_unique(num_items / 10, 1, num_items);

    std::vector<json> items_array;
    items_array.reserve(num_items);

    for (uint64_t i = 1; i <= num_items; i++)
    {
      bool is_original = original_rows.find(i) != original_rows.end();
      
      json item;
      item["key"] = i;
      item["value"] = make_item(is_original);
      items_array.push_back(item);
    }

    auto response = json::from_msgpack(connection->call("TPCC_load_items", items_array));
    // TODO: check response
  }

  void load_warehouse(const ConnPtr& connection, std::uint64_t w_id)
  {
    // TODO: refactor constants
    static uint64_t num_stocks = 100000;

    // Load the warehouse entry
    json warehouse;
    warehouse["key"] = w_id;
    warehouse["value"] = make_warehouse();
    auto response = json::from_msgpack(connection->call("TPCC_load_warehouse", warehouse));
  }

  void load_stocks(const ConnPtr& connection, std::uint64_t w_id)
  {
    static uint64_t num_stocks = 100000; // TODO: refactor constants
    std::unordered_set<uint64_t> original_rows = select_n_unique(num_stocks / 10, 1, num_stocks);

    std::vector<json> stocks_array;
    stocks_array.reserve(num_stocks);

    for (uint64_t i = 1; i <= num_stocks; i++)
    {
      json key;
      key["i_id"] = i;
      key["w_id"] = w_id;

      bool is_original = original_rows.find(i) != original_rows.end();

      json stock;
      stock["key"] = key;
      stock["value"] = make_stock(is_original);
      stocks_array.push_back(stock);
    }

    auto response = json::from_msgpack(connection->call("TPCC_load_stocks", stocks_array));
    // TODO: check response
  }

  void load_district(const ConnPtr& connection, uint64_t d_id, uint64_t w_id)
  {
    json key;
    key["id"] = d_id;
    key["w_id"] = w_id;

    json district;
    district["key"] = key;
    district["value"] = make_district();
    auto response = json::from_msgpack(connection->call("TPCC_load_district", district));
    //TODO: check response
  }
  
  void load_customer(const ConnPtr& connection, uint64_t c_id, uint64_t d_id, uint64_t w_id, bool bad_credit)
  {
    json key;
    key["id"] = c_id;
    key["w_id"] = w_id;
    key["d_id"] = d_id;

    json customer;
    customer["key"] = key;
    customer["value"] = make_customer(c_id, bad_credit);
    auto response = json::from_msgpack(connection->call("TPCC_load_customer", customer));
    //TODO: check response
  }

  void load_history(const ConnPtr& connection, uint64_t c_id, uint64_t d_id, uint64_t w_id)
  {
    json history;
    history["key"] = c_id; // Using c_id as an incrementing ID
    history["value"] = make_history(c_id, d_id, w_id);
    auto response = json::from_msgpack(connection->call("TPCC_load_history", history));
  }

  void load_order(const ConnPtr& connection, uint64_t o_id, uint64_t o_ol_cnt, uint64_t d_id, uint64_t w_id, uint64_t c_id)
  {
    json key;
    key["id"] = o_id;
    key["d_id"] = d_id;
    key["w_id"] = w_id;

    json order;
    order["key"] = key;
    order["value"] = make_order(o_ol_cnt, c_id, o_id >= 2101);
    auto response = json::from_msgpack(connection->call("TPCC_load_order"));
  }

  void load_order_lines(const ConnPtr& connection, uint64_t o_id, uint64_t o_ol_cnt, uint64_t d_id, uint64_t w_id)
  {
    std::vector<json> order_lines_array;
    order_lines_array.reserve(o_ol_cnt);

    for (size_t i = 1; i <= o_ol_cnt; i++) {
      json key;
      key["o_id"] = o_id;
      key["d_id"] = d_id;
      key["w_id"] = w_id;
      key["number"] = i;

      json order_line;
      order_line["key"] = key;
      order_line["value"] = make_order_line(w_id, o_id >= 2101, o_id < 2101);
      order_lines_array.push_back(order_line);
    }

    auto response = json::from_msgpack(connection->call("TPCC_load_order_lines", order_lines_array));
      //TODO: check response
  }

  void load_new_orders(const ConnPtr& connection, uint64_t start, uint64_t end, uint64_t d_id, uint64_t w_id)
  {
    uint64_t amount = end - start + 1;

    std::vector<json> new_orders_array;
    new_orders_array.reserve(amount);

    for (size_t i = start; i <= end; i++) {
      json key;
      key["o_id"] = i;
      key["d_id"] = d_id;
      key["w_id"] = w_id;

      json value;
      value["flag"] = 0;

      json new_order;
      new_order["key"] = key;
      new_order["value"] = value;

      new_orders_array.push_back(new_order);
    }

    auto response = json::from_msgpack(connection->call("TPCC_load_new_orders", new_orders_array));
    //TODO: check response
  }

  /* Individual tuple generators */

  json make_item(bool is_original)
  {
    json item;
    item["im_id"] = rand_range(1, 10000);
    item["price"] = rand_range(100, 10000) / 100;
    item["name"] = rand_astring(14, 24);
    item["data"] = is_original
      ? rand_insert(rand_astring(26, 50), "ORIGINAL")
      : rand_astring(26, 50);
    return item;
  }

  json make_warehouse()
  {
    json warehouse;
    warehouse["name"] = rand_astring(6, 10);
    warehouse["street_1"] = rand_astring(10, 20);
    warehouse["street_2"] = rand_astring(10, 20);
    warehouse["city"] = rand_astring(10, 20);
    warehouse["state"] = rand_astring(2, 2);
    warehouse["zip"] = make_zipcode();
    warehouse["tax"] = rand_range(0, 2000 + 1) / 1000;
    return warehouse;
  }

  json make_stock(bool is_original)
  {
    json stock;
    stock["quantity"] = rand_range(10, 100 + 1);
    stock["ytd"] = 0;
    stock["order_cnt"] = 0;
    stock["remote_cnt"] = 0;
    stock["data"] = is_original
      ? rand_insert(rand_astring(26, 50), "ORIGINAL")
      : rand_astring(26, 50);

    json dist_xx[10];
    for (size_t i = 0; i < 10; i++)
    {
      dist_xx[i] = rand_astring(24, 24);
    }

    stock["dist_xx"] = dist_xx;
    return stock;
  }

  json make_district()
  {
    json district;
    district["name"] = rand_astring(6, 10);
    district["street_1"] = rand_astring(10, 20);
    district["street_2"] = rand_astring(10, 20);
    district["city"] = rand_astring(10, 20);
    district["state"] = rand_astring(2, 2);
    district["zip"] = make_zipcode();
    district["tax"] = rand_range(0, 2000) / 1000;
    district["next_o_id"] = 3001;
    return district;
  }

  json make_customer(uint64_t c_id, bool bad_credit)
  {
    // Time of customer table population (required for c_since)
    std::time_t t = std::time(0);

    json customer;
    customer["last"] = make_customer_last(c_id);
    customer["middle"] = "OE";
    customer["first"] = rand_astring(8, 16);
    customer["street_1"] = rand_astring(10, 20);
    customer["street_2"] = rand_astring(10, 20);
    customer["city"] = rand_astring(10, 20);
    customer["state"] = rand_astring(2, 2);
    customer["zip"] = make_zipcode();
    customer["phone"] = rand_nstring(16, 16);
    customer["since"] = ctime(&t);
    customer["credit"] = bad_credit ? "BC" : "GC";
    customer["credit_lim"] = 50000.00;
    customer["discount"] = rand_range(0, 5001) / 1000;
    customer["balance"] = -10.00;
    customer["ytd_payment"] = 10.00;
    customer["payment_cnt"] = 1;
    customer["delivery_cnt"] = 0;
    customer["c_data"] = rand_astring(300, 500);
    return customer;
  }

  json make_customer_last(uint64_t c_id)
  {
    static const std::vector<std::string> syllables = {"BAR", "OUGHT", "ABLE",
      "PRI", "PRES", "ESE", "ANTI", "CALLY", "ATION", "EING"};

    uint64_t selection = c_id < 1000 ? c_id : nu_rand(255, 0, 999);

    char buffer[4];
    std::snprintf(buffer, sizeof(buffer), "%03lu", selection);
    
    std::string result = "";
    for (int i = 0; i < 4; i++)
    {
      result += syllables[buffer[i]];
    }

    return result;
  }

  json make_history(uint64_t c_id, uint64_t d_id, uint64_t w_id)
  {
    // Current time
    std::time_t t = std::time(0);

    json history;
    history["c_id"] = c_id;
    history["d_id"] = d_id;
    history["w_id"] = w_id;
    history["date"] = ctime(&t);
    history["amount"] = 10.0;
    history["data"] = rand_astring(12, 24);
    return history;
  }

  json make_order(uint64_t o_ol_cnt, uint64_t c_id, bool null_carrier)
  {
    // Current time
    std::time_t t = std::time(0);

    json order;
    order["c_id"] = c_id;
    order["entry_d"] = ctime(&t);
    order["carrier_id"] = null_carrier ? -1 : rand_range(1, 11);
    order["ol_cnt"] = o_ol_cnt;
    order["all_local"] = 1;
    return order;
  }

  json make_order_line(uint64_t w_id, bool null_delivery_d, bool null_amount)
  {
    // Current time
    std::time_t t = std::time(0);

    json order_line;
    order_line["i_id"] = rand_range(1, 100001);
    order_line["supply_w_id"] = w_id;
    order_line["delivery_d"] = null_delivery_d ? "" : ctime(&t);
    order_line["quantity"] = 5;
    order_line["amount"] = null_amount ? 0.0 : rand_range(1, 999999) / 100;
    order_line["dist_info"] = rand_astring(24, 24);
    return order_line;
  }

  std::string make_zipcode()
  {
    return rand_nstring(4, 4) + "11111";
  }

  /* ----- Random Utils ----- */

  /*
    Non-Uniform Random number, NURand[A, x, y], as per TPCC 2.1.6
  */
  template <typename T>
  T nu_rand(T a, T x, T y)
  {
    T c = 0;
    // TODO: implement setting C correctly
    return (((rand_range(0, a) | rand_range(x, y)) + c) % (y - x + 1)) + x;
  }

  /*
    Select n unique numbers from the range [min, max]
  */
  std::unordered_set<uint64_t> select_n_unique(size_t n, uint64_t min, uint64_t max)
  {
    std::unordered_set<uint64_t> unique;

    for (size_t i = 0; i < n; i++)
    {
      uint64_t item;
      do
      {
        item = rand_range(min, max + 1);
      }
      while (unique.find(item) != unique.end());

      unique.emplace(item);
    }

    return unique;
  }

  /*
    Random a-string generator (alphanumeric)
  */
  std::string rand_astring(size_t min_len, size_t max_len)
  {
    constexpr char charset[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    return rand_string(min_len, max_len, charset, sizeof(charset));
  }

  /*
    Random n-string generator (numeric)
  */
  std::string rand_nstring(size_t min_len, size_t max_len)
  {
    constexpr char charset[] = "0123456789";

    return rand_string(min_len, max_len, charset, sizeof(charset));
  }

  std::string rand_string(int min_len, int max_len, const char* charset, int charset_len)
  {
    int length = rand_range(min_len, max_len + 1);
    std::string result = "";

    for (int i = 0; i < length; i++)
    {
      int index = rand_range(0, charset_len);
      result += charset[index];
    }

    return result;
  }

  /*
    Places the given substring in a random position in the input
  */
  std::string rand_insert(std::string input, std::string substring)
  {
    int max_index = input.length() - substring.length();
    int start_index = rand_range(0, max_index);

    return input.replace(start_index, substring.length(), substring);
  }

  /*
    Returns array with a randomized sequence of numbers in the range min to max
  */
  int* permutation(uint64_t min, uint64_t max)
  {
    int size = max - min + 1;
    int* results = new int[size];

    for (int i = 0; i < size; i++)
    {
      int index;
      do
      {
        index = rand_range(0, size);
      } while (results[index] != 0);

      results[index] = i;
    }

    return results;
  }

public:
  TpccClient() : Base("Tpcc_ClientCpp") {}

  void setup_parser(CLI::App& app) override
  {
    Base::setup_parser(app);

    app.add_option("--num_warehouses", num_warehouses);
  }

};

int main(int argc, char** argv)
{
  TpccClient client;
  CLI::App cli_app{"TPCC Client"};
  client.setup_parser(cli_app);
  CLI11_PARSE(cli_app, argc, argv);

  client.run();

  return 0;
}