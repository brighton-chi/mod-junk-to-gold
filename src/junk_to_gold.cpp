#include "junk_to_gold.h"

void JunkToGold::OnLootItem(Player* player, Item* item, uint32 count, ObjectGuid /*lootguid*/)
{
    if(IsEnabled)
    {
        if (!item || !item->GetTemplate())
            return;

        if(isLogEnabled)
            LOG_INFO("server.world", "Player {} Looted Item: {} (Count: {})", player->GetName(), item->GetTemplate()->Name1, count);

        Process(player, item, count);
    }
}

void JunkToGold::OnQuestRewardItem(Player* player, Item* item, uint32 count)
{
    if(IsEnabled)
    {
        if (!item || !item->GetTemplate())
            return;

        if(sConfigMgr->GetOption<bool>("JunkToGold.Logging.Enable", true))
            LOG_INFO("server.world", "Player {} Recieved Quest Reward: {} (Count: {})", player->GetName(), item->GetTemplate()->Name1, count);

        Process(player, item, count);
    }
}

void JunkToGold::Process(Player* player, Item* item, uint32 count)
{
    static const std::array<std::string, 8> QUALITY_ARRAY = {
        "JunkToGold.Quality.0", "JunkToGold.Quality.1", "JunkToGold.Quality.2",
        "JunkToGold.Quality.3", "JunkToGold.Quality.4", "JunkToGold.Quality.5",
        "JunkToGold.Quality.6", "JunkToGold.Quality.7"
    };

    uint32 quality = item->GetTemplate()->Quality;

    if (quality < qualityKeys.size() && sConfigMgr->GetOption<bool>(QUALITY_ARRAY[quality], true))
    {
        SendTransactionInformation(player, item, count);
        player->ModifyMoney(item->GetTemplate()->SellPrice * count);
        player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
    }
}

void JunkToGold::SendTransactionInformation(Player* player, Item* item, uint32 count)
{
    std::string name;

    if (count > 1)
    {
        name = Acore::StringFormat("|cff9d9d9d|Hitem:{}::::::::80:::::|h[{}]|h|rx{}", item->GetTemplate()->ItemId, item->GetTemplate()->Name1, count);
    }
    else
    {
        name = Acore::StringFormat("|cff9d9d9d|Hitem:{}::::::::80:::::|h[{}]|h|r", item->GetTemplate()->ItemId, item->GetTemplate()->Name1);
    }

    uint32 money = item->GetTemplate()->SellPrice * count;
    uint32 gold = money / GOLD;
    uint32 silver = (money % GOLD) / SILVER;
    uint32 copper = (money % GOLD) % SILVER;

    std::string info;

    if (money < SILVER)
    {
        info = Acore::StringFormat("{} sold for {} copper.", name, copper);
    }
    else if (money < GOLD)
    {
        if (copper > 0)
        {
            info = Acore::StringFormat("{} sold for {} silver and {} copper.", name, silver, copper);
        }
        else
        {
            info = Acore::StringFormat("{} sold for {} silver.", name, silver);
        }
    }
    else
    {
        if (copper > 0 && silver > 0)
        {
            info = Acore::StringFormat("{} sold for {} gold, {} silver and {} copper.", name, gold, silver, copper);
        }
        else if (copper > 0)
        {
            info = Acore::StringFormat("{} sold for {} gold and {} copper.", name, gold, copper);
        }
        else if (silver > 0)
        {
            info = Acore::StringFormat("{} sold for {} gold and {} silver.", name, gold, silver);
        }
        else
        {
            info = Acore::StringFormat("{} sold for {} gold.", name, gold);
        }
    }

	if(isLogEnabled)
    {
        LOG_INFO("server.world", "{}", info);
    }

    ChatHandler(player->GetSession()).SendSysMessage(info);
}
