#ifndef ENUMS_BLOCKS_HPP
#define ENUMS_BLOCKS_HPP

#include <memory>

#include "Globals.hpp"

namespace Components {
    // base class for all components
    class Component {
    public:
        virtual ~Component() = default;
    };

    // defines if a block needs to be rendered, if yes, then its texture Id
    class Render : virtual public Component {
    public:
        Render(uint8_t textureId) : TextureId(textureId) {}

        uint8_t TextureId;
    };

    // defines if a block is collideable
    class Collision : virtual public Component {
    public:
    };

    // defines if a block is enterable
    class Warp : virtual public Component {
    public:
    };

    // defines if a block is hitable
    class Hitable : virtual public Component {
    public:
    };

    // defines if a block is breakable
    class Breakable : virtual public Component, public Hitable {
    public:
    };

    // defines if a block spawns an item
    class Item : virtual public Component, public Hitable {
    public:
        Item(uint8_t itemId) : ItemId(itemId) {}

        uint8_t ItemId;
    };

    // defines if a block is hidden
    class Hidden : virtual public Component, public Collision, public Item {
    public:
        Hidden(uint8_t itemId) : Item(itemId) {}
    };
}

namespace Blocks {
    // base class for all blocks
    class Block {
    public:
        virtual ~Block() = default;
    };

    // renderable
    class Renderable : virtual public Block, public Components::Render {
    public:
        Renderable(uint8_t textureId) : Components::Render(textureId) {}
    };

    // collideable
    class Collideable : virtual public Block, public Components::Collision {
    public:
        Collideable() {}
    };

    // renderable, collideable
    class RenderableCollideable : virtual public Block, public Components::Render, public Components::Collision {
    public:
        RenderableCollideable(uint8_t textureId) : Components::Render(textureId) {}
    };

    // renderable, collideable, enterable
    class RenderableCollideableEnterable : virtual public Block, public Components::Render, public Components::Collision, public Components::Warp {
    public:
        RenderableCollideableEnterable(uint8_t textureId) : Components::Render(textureId) {}
    };

    // hidden
    class Hidden : virtual public Block, public Components::Hidden {
    public:
        Hidden(uint8_t itemId) : Components::Hidden(itemId) {}
    };

    // renderable, collideable, breakable
    class RenderableCollideableBreakable : virtual public Block, public Components::Render, public Components::Collision, public Components::Breakable {
    public:
        RenderableCollideableBreakable(uint8_t textureId) : Components::Render(textureId) {}
    };

    // renderable, collideable, has item
    class RenderableCollideableItem : virtual public Block, public Components::Render, public Components::Collision, public Components::Item {
    public:
        RenderableCollideableItem(uint8_t textureId, uint8_t itemId) : Components::Render(textureId), Components::Item(itemId) {}
    };

    // coin
    class Coin : virtual public Block, public Components::Render {
    public:
        Coin() : Components::Render(gbl::TextureId::Block::Coin) {}
    };

    // flag
    class Flag : virtual public Block, public Components::Render {
    public:
        Flag(uint8_t textureId) : Components::Render(textureId) {}
    };

    // jump spring trigger
    class JumpSpringTrigger : virtual public Block, public Components::Collision {
    public:
        JumpSpringTrigger() {}
    };
}

#define GetComponent(var, name) dynamic_cast<name*>(var)
#define GetIf(var, name) dynamic_cast<name*>(var)

#endif // !ENUMS_BLOCKS_HPP