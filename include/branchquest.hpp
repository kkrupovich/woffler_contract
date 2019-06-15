#include <entity.hpp>

namespace Woffler {
  using namespace eosio;
  using std::string;

  namespace BranchQuest {
    //branch-to-quest references
    typedef struct
    [[eosio::table("brquests"), eosio::contract("woffler")]]      
    wflbrquest {
      uint64_t id;
      uint64_t idbranch;
      uint64_t idquest;
      name owner;
      
      uint64_t primary_key() const { return id; }
    } wflbrquest;

    typedef multi_index<"brquest"_n, wflbrquest> brquests;  

    class DAO: public Accessor<brquests, wflbrquest, brquests::const_iterator, uint64_t>  {
      
      public:
    
      DAO(brquests& _brquests, const uint64_t& idbrquest): 
        Accessor<brquests, wflbrquest, brquests::const_iterator, uint64_t>::Accessor(_brquests, idbrquest) {}

      DAO(brquests& _brquests, const brquests::const_iterator& itr): 
        Accessor<brquests, wflbrquest, brquests::const_iterator, uint64_t>::Accessor(_brquests, itr) {}
    
      static uint64_t keyValue(const uint64_t& idbrquest) {
        return idbrquest;
      }
    };

    class BranchQuest: Entity<brquests, DAO, uint64_t, wflbrquest> {
      public:
      BranchQuest(const name& self, const uint64_t& idbrquest) : Entity<brquests, DAO, uint64_t, wflbrquest>(self, idbrquest) {}

    };
  }
}


