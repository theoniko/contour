/**
 * This file is part of the "libterminal" project
 *   Copyright (c) 2019 Christian Parpart <christian@parpart.family>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <crispy/TextShaper.h>
#include <crispy/reference.h>
#include <iostream>

using namespace std;

namespace crispy::text {

constexpr unsigned MaxInstanceCount = 10;
constexpr unsigned MaxTextureDepth = 10;
constexpr unsigned MaxTextureSize = 1024;

TextShaper::TextShaper() :
    renderer_{},
    monochromeAtlas_{
        MaxInstanceCount,
        min(MaxTextureDepth, renderer_.maxTextureDepth()),
        min(MaxTextureSize, renderer_.maxTextureSize()),
        min(MaxTextureSize, renderer_.maxTextureSize()),
        renderer_.scheduler(),
        "monochromeAtlas"
    },
    colorAtlas_{
        MaxInstanceCount,
        min(MaxTextureDepth, renderer_.maxTextureDepth()),
        min(MaxTextureSize, renderer_.maxTextureSize()),
        min(MaxTextureSize, renderer_.maxTextureSize()),
        renderer_.scheduler(),
        "colorAtlas",
    }
{
}

TextShaper::~TextShaper()
{
}

void TextShaper::setProjection(QMatrix4x4 const& _projection)
{
    renderer_.setProjection(_projection);
}

void TextShaper::render(QPoint _pos,
                        vector<Font::GlyphPosition> const& _glyphPositions,
                        QVector4D const& _color)
{
    for (Font::GlyphPosition const& gpos : _glyphPositions)
        if (optional<DataRef> const ti = getTextureInfo(GlyphId{gpos.font, gpos.glyphIndex}); ti.has_value())
            renderTexture(_pos,
                          _color,
                          get<0>(*ti).get(),
                          get<1>(*ti).get(),
                          gpos);
}

optional<TextShaper::DataRef> TextShaper::getTextureInfo(GlyphId const& _id)
{
    TextureAtlas& atlas = _id.font.get().hasColor()
        ? colorAtlas_
        : monochromeAtlas_;

    return getTextureInfo(_id, atlas);
}

optional<TextShaper::DataRef> TextShaper::getTextureInfo(GlyphId const& _id,
                                                         TextureAtlas& _atlas)
{
    if (optional<DataRef> const dataRef = _atlas.get(_id); dataRef.has_value())
        return dataRef;

    Font& font = _id.font.get();
    Font::Glyph fg = font.loadGlyphByIndex(_id.glyphIndex);

    auto metadata = Glyph{};
    metadata.advance = _id.font.get()->glyph->advance.x >> 6;
    metadata.bearing = QPoint(font->glyph->bitmap_left, font->glyph->bitmap_top);
    metadata.descender = (font->glyph->metrics.height >> 6) - font->glyph->bitmap_top;
    metadata.height = static_cast<unsigned>(font->height) >> 6;
    metadata.size = QPoint(static_cast<int>(font->glyph->bitmap.width), static_cast<int>(font->glyph->bitmap.rows));

    std::cout << "Atlas.insert: "
        << fmt::format("({}x{})", fg.width, fg.height)
        << " into " << _atlas << endl;
    return _atlas.insert(_id, fg.width, fg.height, move(fg.buffer), move(metadata));
}

void TextShaper::renderTexture(QPoint const& _pos,
                               QVector4D const& _color,
                               atlas::TextureInfo const& _textureInfo,
                               Glyph const& _glyph,
                               Font::GlyphPosition const& _gpos)
{
    unsigned const px = _pos.x() + _gpos.x;
    unsigned const py = _pos.y() + _gpos.y;

    auto const x = static_cast<unsigned>(px + _glyph.bearing.x());
    auto const y = static_cast<unsigned>(py + _gpos.font.get().baseline() - _glyph.descender);
    auto const z = 0u;

    renderer_.scheduler().renderTexture({_textureInfo, x, y, z, _color});
}

void TextShaper::execute()
{
    renderer_.execute();
}

void TextShaper::clearCache()
{
    monochromeAtlas_.clear();
    colorAtlas_.clear();
}

} // end namespace