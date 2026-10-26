#include <Geode/modify/CustomizeObjectLayer.hpp>
#include <Geode/modify/TextGameObject.hpp>
#include "settings.hpp"

using namespace geode::prelude;

class $modify(CustomizeObjectLayer) {
    bool init(GameObject* object, CCArray* objects) {
        if (!CustomizeObjectLayer::init(object, objects)) return false;
        
        if (settings::enabled && settings::inputBypass && m_textInput) {
            m_textInput->m_maxLabelLength = std::numeric_limits<int>::max();
            m_textInput->m_allowedChars = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
        }

        return true;
    }

    void textChanged(CCTextInputNode* node) {
        CustomizeObjectLayer::textChanged(node);

        if (settings::enabled && settings::insertNewlines && node && node == m_textInput) {
            m_textInput->setString(string::replace(m_textInput->getString(), "\\n", "\n"));
        }
    }
};

class $modify(TextGameObject) {
    // from arctics vibe decomp
    static bool validText(std::string_view str) {
        if (str.contains('\0')) {
            return false;
        }

        size_t i = 0;
        while (i < str.size()) {
            unsigned char c = static_cast<unsigned char>(str[i]);

            int extra = c < 0x80 ? 0 : (c >> 5) == 6 ? 1 : (c >> 4) == 14 ? 2 : (c >> 3) == 30 ? 3 : -1;

            if (extra < 0 || i + extra >= str.size()) {
                return false;
            }
            for (int k = 1; k <= extra; k++) {
                if ((static_cast<unsigned char>(str[i + k]) & 0xc0) != 0x80) {
                    return false;
                }
            }

            i += extra + 1;
        }

        return true;
    }

    static void onModify(auto& self) {
        (void)self.setHookPriority("TextGameObject::updateTextObject", Priority::Replace);
    }

    void updateTextObject(std::string text, bool defaultFont) {
        if (!settings::enabled) {
            return TextGameObject::updateTextObject(text, defaultFont);
        }

        const bool valid = validText(text);

        if (valid) {
            if (text.empty() || text == " ") {
                text = "A";
            }
        }

        m_text = text;
        const auto hasChildren = this->getChildrenCount();
        const auto color = hasChildren ? this->getChildrenExt<CCSprite>().front()->getColor() : ccWHITE;

        this->removeAllChildrenWithCleanup(true);

        // @geode-ignore(unknown-resource)
        std::string font{"bigFont.fnt"};

        if (!defaultFont) {
            const int index = std::clamp(GameManager::sharedState()->m_loadedFont, 0, 59);

            GameManager::sharedState()->loadFont(index);

            if (index) {
                font = fmt::format("gjFont{:02}.fnt", index);
            }
        }

        auto letters = CCArray::create();
        auto label = CCLabelBMFont::createBatched(
            valid ? text.c_str() : "", font.c_str(), letters, m_kerning
        );

        this->setContentSize(label->getContentSize());
        label->setPosition(this->convertToNodeSpace(CCPointZero));

        for (auto letter : CCArrayExt<CCSprite>(letters)) {
            if (m_isSelected && !hasChildren) {
                letter->setColor(color);
            }

            this->addChild(letter);
        }
        
        m_hasCustomChild = true;
        m_isObjectRectDirty = true;
        m_isOrientedBoxDirty = true;
        m_isDirty = true;
        this->setDirty(true);
        m_bDontDraw = true;
    }
};