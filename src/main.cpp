#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

// Форматирование числа с запятыми
static std::string formatNumberWithCommas(int number) {
    std::string s = std::to_string(number);
    int n = s.length();

    for (int i = n - 3; i > 0; i -= 3) {
        s.insert(i, ",");
    }
    return s;
}

class $modify(MyProfilePage, ProfilePage) {
public:
    bool init(int accountID, bool ownProfile) {
        if (!ProfilePage::init(accountID, ownProfile)) return false;
        return true;
    }

    // хук на кнопку Update — чистим свои ноды перед обновлением профиля
    void onUpdate(CCObject* sender) {
        auto layer = static_cast<CCLayer*>(this->m_mainLayer);
        if (auto statsMenu = layer ? layer->getChildByID("stats-menu") : nullptr) {
            const std::vector<std::string> statsIDs = {
                "stars","moons","diamonds","coins","user-coins","demons","creator-points"
            };

            for (auto const& id : statsIDs) {
                if (auto node = statsMenu->getChildByID(id + "-label-container")) {
                    node->removeFromParent();
                }
            }
        }

        ProfilePage::onUpdate(sender);
    }

    void loadPageFromUserInfo(GJUserScore* a1) {
    // ваниль
    ProfilePage::loadPageFromUserInfo(a1);
      
    auto layer = static_cast<CCLayer*>(this->m_mainLayer);
    if (!layer) return;

    auto statsMenu = layer->getChildByID("stats-menu");
    if (!statsMenu) return;

    float posY = 5.0f;

    auto starsIcon  = statsMenu->getChildByID("stars-icon");
    auto moonsIcon  = statsMenu->getChildByID("moons-icon");
    auto demonsIcon = statsMenu->getChildByID("demons-icon");

    bool hasCreatorPoints = (a1->m_creatorPoints != 0); 

    // какие статы рисуем
    const std::vector<std::string> statsIDs = {
        "stars","moons", "diamonds", "coins", "user-coins", "demons", "creator-points"
    };

    // ---------- ВАЖНОЕ: находим центр ванильной линии ----------
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    int   found = 0;

    for (auto const& id : statsIDs) {
        if (auto cont = statsMenu->getChildByID(id + "-label-container")) {
            float x = cont->getPositionX();
            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            found++;
        }
    }

    // если вдруг вообще ничего не нашли — просто выходим
    if (found == 0) return;

    float vanillaCenterX = (minX + maxX) / 2.f;
    // -----------------------------------------------------------

    // чистим старые контейнеры и иконки
    for (const auto& id : statsIDs) {
        if (statsMenu->getChildByID(id + "-label-container") != nullptr) {
            statsMenu->removeChildByID(id + "-label-container");
        }
    }
    statsMenu->removeChildByID("stars-label-container");
    statsMenu->removeChildByID("diamonds-icon");
    statsMenu->removeChildByID("coins-icon");
    statsMenu->removeChildByID("user-coins-icon");
    statsMenu->removeChildByID("creator-points-icon");

    // иконки и значения
    std::vector<std::string> iconFrames = {
        "GJ_starsIcon_001.png", "GJ_moonsIcon_001.png", "GJ_diamondsIcon_001.png",
        "GJ_coinsIcon_001.png", "GJ_coinsIcon2_001.png", "GJ_hammerIcon_001.png",
        "GJ_hammerIcon_001.png"
    };
    std::vector<int> values = {
        a1->m_stars, a1->m_moons, a1->m_diamonds, a1->m_secretCoins,
        a1->m_userCoins, a1->m_creatorPoints, a1->m_creatorPoints
    };

    float spacingX = 50.0f; 

    // считаем, сколько реально статов будет (без creator-points, если их нет)
    int visibleStatsCount = 0;
    for (auto const& id : statsIDs) {
        if (id == "creator-points" && !hasCreatorPoints) continue;
        visibleStatsCount++;
    }
    if (visibleStatsCount <= 0) return;

    // ширина нашего ряда и его левый край: центр = центр ванильной линии
    float totalWidth = (visibleStatsCount - 1) * spacingX;
    float baseX = vanillaCenterX - totalWidth / 2.f;

    int iconIterator = 0;
    
    for (size_t i = 0; i < statsIDs.size(); ++i) {
        if (statsIDs[i] == "creator-points" && !hasCreatorPoints) {
            continue;
        }

        iconIterator++;

        auto container = CCNode::create();
        container->setID(statsIDs[i] + "-label-container");
 
        std::string valueStr = formatNumberWithCommas(values[i]);
        auto label = CCLabelBMFont::create(valueStr.c_str(), "bigFont.fnt");
        label->setID(statsIDs[i] + "-label");

        float scaleFactor = 0.65f - (valueStr.length() - 1) * 0.15f; 
        label->setScale(std::max(scaleFactor, 0.285f)); 

        auto icon = CCSprite::createWithSpriteFrameName(iconFrames[i].c_str());
        if (starsIcon) {
            icon->setScale(starsIcon->getScale());
        }
        icon->setID(statsIDs[i] + "-icon");
        
        float labelX = baseX + (iconIterator - 1) * spacingX + 10; 
        float iconX  = labelX + 25; 

        if (iconIterator == 1 && starsIcon) {
            label->setPosition(ccp(labelX, posY));
            container->addChild(label);
            starsIcon->setPosition(ccp(iconX, posY));
        }
        else if (iconIterator == 2 && moonsIcon) {
            label->setPosition(ccp(labelX, posY));
            container->addChild(label);
            moonsIcon->setPosition(ccp(iconX, posY));
        }
        else if (iconIterator == 6 && demonsIcon) {
            std::string valueStrD = formatNumberWithCommas(a1->m_demons);
            auto demonsLabel = CCLabelBMFont::create(valueStrD.c_str(), "bigFont.fnt");
          
            float scaleFactorD = 0.65f - (valueStrD.length()) * 0.25f; 
            demonsLabel->setScale(std::max(scaleFactorD, 0.285f));

            demonsLabel->setPosition(ccp(labelX, posY));
            container->addChild(demonsLabel);
            demonsIcon->setPosition(ccp(iconX, posY));
        }
        else {
            label->setPosition(ccp(labelX, posY));
            icon->setPosition(ccp(iconX, posY));
            container->addChild(icon);
            container->addChild(label);
        }

        statsMenu->addChild(container);
    }
}

};
