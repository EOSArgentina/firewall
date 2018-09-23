#include <eosiolib/eosio.hpp>
#include "firewall.hpp"

//#include <boost/regex.hpp>
using namespace std;
using namespace eosio;

#define WHITELIST 1
#define BLACKLIST 2
#define MEMO 3
#define LIMIT 4

size_t split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}

asset asset_from_string(string txt) {
  std::vector<std::string> v;
  split( txt, v, ' ' );
  eosio::asset ass;
  ass.amount = std::stol(v[0]);
  ass.symbol = string_to_symbol(4,v[1].c_str());
  return ass;
}

void firewall::transfer( account_name from,
               account_name to,
               asset quantity,
               string memo ){

  bool outgoing = (from == _self);
  rules ds_rules =rules(_self,_self);
  account_name third_party = outgoing ? to : from;

    for(auto& item : ds_rules)
    {
      check_filter(item.filter_type, item.parameter, item.value, third_party, quantity, memo);
    }

//TODO : match incomming and outgoing

  }

  void firewall::check_filter(uint32_t ftype, string param, string val,account_name third_party, eosio::asset quantity, string memo) {

    switch(ftype){
    case WHITELIST:
      return check_whitelist(third_party, val);
    case BLACKLIST:
      return  check_blacklist(third_party, val);
    case MEMO:
      return check_memo( val, memo );
    case LIMIT:
      return check_limit(quantity, val);
    default:
      eosio_assert(false, "Filter type can only be whitelist, blacklist, limit or memo.");
    };
  }

  void firewall::check_whitelist( account_name third_party, string authorized )
  {
    eosio_assert((third_party == string_to_name(authorized.c_str())), "Account invalid or not whilisted.");

  }
  void firewall::check_blacklist( account_name third_party, string blocked )
  {
    eosio_assert((third_party != string_to_name(blocked.c_str())),"Account invalid or not whilisted.");
  }
  void firewall::check_limit( asset transfer_amount, string limit_amount)
  {
    eosio_assert((asset_from_string(limit_amount) < transfer_amount), "amount exceed the limit for tx.");
  }

  void firewall::check_memo( string regex, string memo)
  {
    //check memo well formated
    eosio_assert( !memo.empty(), "Memo can't be empty." );
    //TODO: get regex de la tabla
    eosio_assert(check_regex(regex, memo), "Memo didnt satisfied rgex rule");
  }


//reg is the string taken from the table
//memo is the memo
bool firewall::check_regex(string reg, string memo){
//     static const boost::regex e(reg);
//     boost::cmatch result;
//     if (boost::regex_match(memo, results, e))
//     {
      return true;
//     }else{
//       return false;
//     }
}

  void firewall::addrule(string param, string ftype, string val)
  {
    rules ds_rules =rules(_self,_self);
    eosio_assert(param == "third_party" || param == "memo" || param == "amount", "Param can only be third_party, memo, amount.");
    eosio_assert(ftype == "whitelist" || ftype == "blacklist" || ftype == "limit" || ftype == "memo", "Filter type can only be whitelist, blacklist, limit or memo.");
    eosio_assert(val.size() <= 128, "Filter value has more than 128 bytes.");

    uint32_t itype;
    if (ftype == "whitelist") {
      itype = WHITELIST;
    } else if ( ftype == "blacklist" ) {
      itype = BLACKLIST;
    } else if ( ftype == "limit") {
      itype = LIMIT;
    } else if ( ftype == "memo") {
      itype = MEMO;
    }

    ds_rules.emplace(_self, [&](auto& r)
    {
      r.id = ds_rules.available_primary_key();
      r.parameter =  param;
      r.filter_type = itype;
      r.value = val;
    });
  }

  void firewall::delrule(uint64_t id)
  {
    rules ds_rules =rules(_self,_self);
    std::vector<uint64_t> keysForDeletion;
    for(auto& item : ds_rules)
    {
        if (item.id == id)
        {
            keysForDeletion.push_back(item.id);
        }
    }
    // now delete each item for that poll
    for (uint64_t key : keysForDeletion)
    {
        auto itr = ds_rules.find(key);
        if (itr != ds_rules.end())
        {
          ds_rules.erase(itr);
        }
    }
  }



  #define EOSIO_ABI_EX(TYPE, MEMBERS) extern "C" { \
      void apply(uint64_t receiver, uint64_t code, uint64_t action) { \
        if (action == N(eosio.transfer)){ \
          eosio_assert(code == N(eosio),"onerror action's are only valid from the \"eosio\" system account"); \
        } \
        if (action == N(onerror)) { \
          eosio_assert(code == N(eosio),"onerror action's are only valid from the \"eosio\" system account"); \
        } \
        auto self = receiver; \
        TYPE thiscontract(self); \
        switch (action) { EOSIO_API(TYPE, MEMBERS) }; \
      } \
  } \

  EOSIO_ABI_EX(firewall, (transfer));
