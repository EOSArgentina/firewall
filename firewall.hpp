#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/multi_index.hpp>

class firewall : public eosio::contract {

	public:
    firewall(account_name self) : eosio::contract(self) {}

	 	// @abi action
    void transfer( account_name from, account_name to,eosio::asset quantity,std::string memo );

   	// @abi action
    void add_rule(std::string param, std::string ftype,std::string val);

    // @abi action
    void del_rule(uint64_t id);

  private:

    void check_filter(uint32_t ftype, std::string param, std::string val,account_name third_party, eosio::asset quantity, std::string memo);
    void check_whitelist( account_name third_party, std::string authorized );
    void check_blacklist( account_name third_party, std::string blocked );
    void check_limit( eosio::asset transfer_amount, std::string limit_amount);
    void check_memo( std::string regex, std::string memo);
    bool check_regex(std::string reg, std::string memo);

    struct rule {
			uint64_t 	 id;
			std::string       parameter; // from, to, greater, memo
			uint32_t       filter_type; // whitelist, blacklist, greater than,memo.
			std::string       value;  //account_name to blacklist, amount of eos, wildcards for default
			auto get_parameter() const { return parameter; }
			auto get_filter_type() const { return filter_type; }
			auto get_value() const { return value; }
			auto primary_key() const { return id; }
			EOSLIB_SERIALIZE( rule, (id)( parameter )( filter_type )( value ) );
		};

		typedef eosio::multi_index<N(rule), rule> rules;
};
