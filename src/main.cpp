#include <utils.hpp>
#include <constants.hpp>
#include "player.cpp"
#include "channel.cpp"
#include "branch.cpp"
#include "level.cpp"
#include "branchmeta.cpp"
#include "stake.cpp"
#include "quest.cpp"
#include "branchquest.cpp"

namespace Woffler {
  using namespace eosio;
  using std::string;

  class
  [[eosio::contract("woffler")]]
  woffler : public contract {
    public:
    using contract::contract;
    woffler(name receiver, name code, datastream<const char*> ds):
      contract(receiver, code, ds) {}
    
    #pragma region ** Handlers: **
    
    [[eosio::on_notify("eosio.token::transfer")]]
    void transferHandler(name from, name to, asset amount, string memo) {
      auto self = get_self();
      print("Processing transfer from: ", name{from}, " to: ", name{to}, " amount : ", asset{amount}, "\n");
      check(
        amount.symbol.code() == Const::acceptedCurr,
        "Only " + Const::acceptedCurr.to_string() + " transfers allowed"
      );

      if (to == self) { //deposit
        Player::Player player(self, from);
        if (!player.isPlayer()) {
          player.createPlayer(self, self);
        }
        player.addBalance(amount, self);
      }
    }

    using transferAction = action_wrapper<"transfer"_n, &woffler::transferHandler>;
    
    #pragma endregion

    #pragma region ** Contract (on-off boarding and deposit/withdraw): **
    
    //signup new player with custom sales channel (via referral link)
    ACTION signup(name account, name referrer) {
      require_auth(account);

      Player::Player player(get_self(), account);
      player.createPlayer(account, referrer);//player pays RAM to store his record
    }

    //withdraw player's funds to arbitrary account (need auth by player)
    ACTION withdraw (name from, name to, asset amount, const string& memo) {
      require_auth(from);

      auto self = get_self();
      Player::Player player(self, from);
      player.subBalance(amount, from);

      // Inline transfer
      const auto& contract = name("eosio.token");
      transferAction t_action(contract, {self, "active"_n});
      t_action.send(self, to, amount, memo);
    }

    //forget player (without balance check yet, TBD!)
    ACTION forget(name account) {
      require_auth(account);

      Player::Player player(get_self(), account);
      player.rmAccount();
    }
    
    #pragma endregion
    
    #pragma region ** Branch presets (wflBranchMeta): **

    //create meta for root branch(es) - active balance must cover at least.
    //owner pays for ram to avoid spamming via branch meta creation.
    //only owner can modify branch metadata.
    ACTION brnchmeta(name owner, BranchMeta::wflbrnchmeta meta) {
        require_auth(owner);

        BranchMeta::BranchMeta branchmeta(get_self(), meta.id);
        branchmeta.upsertBranchMeta(owner, meta);
      }

    //remove branch meta owned
    ACTION rmbrmeta(name owner, uint64_t idmeta) {
      require_auth(owner);

      BranchMeta::BranchMeta branchmeta(get_self(), idmeta);
      branchmeta.removeBranchMeta(owner);
    }

    #pragma endregion

    #pragma region ** wflBranch**
    
    //create root branch after meta is created/selected from existing
    //register pot value as owner's stake in root branch created
    ACTION branch(name owner, uint64_t idmeta, asset pot) {
      require_auth(owner);
      
      Branch::Branch branch(get_self(), 0);
      branch.createBranch(owner, idmeta, pot);      
    }
    
    //create root level with all branch stake (from all owners)
    //generate cells for root level
    ACTION rootlvl(name owner, uint64_t idbranch) {
      require_auth(owner);
      
      Branch::Branch branch(get_self(), idbranch);
      branch.createRootLevel(owner);
    }

    //DEBUG actions for branch generation debug 
    ACTION setrootlvl(name owner, uint64_t idbranch, uint64_t idrootlvl) {
      require_auth(owner);
      auto self = get_self();
      check(
        owner == self,
        string("Debug mode available only to contract owner: ") + self.to_string()
      );

      Branch::Branch branch(self, idbranch);
      branch.setRootLevel(owner, idrootlvl);
    }

    #pragma endregion

    #pragma region ** wflLevel **

    //generate cells for a given level and mark level unlocked if compatible green/red set found
    ACTION unlocklvl(name owner, uint64_t idlevel) {
      require_auth(owner);

      Level::Level level(get_self(), idlevel);
      level.unlockLevel(owner);
    }
    
    #pragma endregion

    #pragma region ** wflPlayer **
        
    //set current root branch for player and position at 1st level
    ACTION switchbrnch(name account, uint64_t idbranch) {
      require_auth(account);

      Player::Player player(get_self(), account);      
      player.switchBranch(idbranch);//position player in root level of the branch
    }
        
    //use try to change position in current level from safe to green. last try will change position automatically
    ACTION tryturn(name account) {
      require_auth(account);

      Player::Player player(get_self(), account);
      player.tryTurn();
    }
    
    //commit position change in current level
    ACTION committurn(name account) {
      require_auth(account);
      
      Player::Player player(get_self(), account);
      player.commitTurn();

    }

    
    #pragma endregion
  };
}