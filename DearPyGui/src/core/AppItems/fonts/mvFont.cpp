#include "mvFont.h"
#include "mvLog.h"
#include "mvItemRegistry.h"
#include "mvPythonExceptions.h"
#include "mvUtilities.h"
#include "textures/mvStaticTexture.h"
#include "mvToolManager.h"
#include "mvFontRangeHint.h"
#include "mvFontManager.h"

namespace Marvel {

	void mvFont::InsertParser(std::map<std::string, mvPythonParser>* parsers)
	{

		mvPythonParser parser(mvPyDataType::UUID, "Undocumented function", { "Textures", "Widgets" });
		mvAppItem::AddCommonArgs(parser, (CommonParserArgs)(
			MV_PARSER_ARG_ID |
			MV_PARSER_ARG_PARENT)
		);

		parser.addArg<mvPyDataType::String>("file");
		parser.addArg<mvPyDataType::Integer>("size");
		parser.addArg<mvPyDataType::Bool>("default_font", mvArgType::KEYWORD_ARG, "False");

		parser.finalize();

		parsers->insert({ s_command, parser });
	}

	mvFont::mvFont(mvUUID uuid)
		:
		mvAppItem(uuid)
	{

	}


	bool mvFont::isParentCompatible(mvAppItemType type)
	{
		if (type == mvAppItemType::mvFontRegistry) return true;

		mvThrowPythonError(1000, "Drawing item parent must be a drawing.");
		MV_ITEM_REGISTRY_ERROR("Drawing item parent must be a drawing.");
		assert(false);
		return false;
	}

	void mvFont::customAction()
	{
		ImGuiIO& io = ImGui::GetIO();

		m_fontPtr = io.Fonts->AddFontFromFileTTF(m_file.c_str(), m_size, 
			nullptr, m_ranges.Data);

		if (m_fontPtr == nullptr)
		{
			mvThrowPythonError(1000, "Font file could not be found");
			io.Fonts->Build();
			return;
		}

		// handled by char remaps
		//for (auto& item : font.charRemaps)
		//	m_fontPtr->AddRemapChar(item.first, item.second);

		io.Fonts->Build();

		if(m_default)
			io.FontDefault = m_fontPtr;
	}

	void mvFont::draw(ImDrawList* drawlist, float x, float y)
	{

		ImVector<ImWchar> ranges;
		ImFontGlyphRangesBuilder builder;

		static ImFontAtlas atlas;

		// check hints
		for (const auto& hint : m_children[0])
		{
			int hintSelection = static_cast<mvFontRangeHint*>(hint.get())->getHint();

			switch (hintSelection)
			{
			case 1:
				builder.AddRanges(atlas.GetGlyphRangesJapanese());
				break;
			case 2:
				builder.AddRanges(atlas.GetGlyphRangesKorean());
				break;
			case 3:
				builder.AddRanges(atlas.GetGlyphRangesChineseFull());
				break;
			case 4:
				builder.AddRanges(atlas.GetGlyphRangesChineseSimplifiedCommon());
				break;
			case 5:
				builder.AddRanges(atlas.GetGlyphRangesCyrillic());
				break;
			case 6:
				builder.AddRanges(atlas.GetGlyphRangesThai());
				break;
			case 7:
				builder.AddRanges(atlas.GetGlyphRangesVietnamese());
				break;
			default:
				builder.AddRanges(atlas.GetGlyphRangesDefault());
			}

		}

		// handled by custom ranges
		//for (const auto& range : fontGlyphRangeCustom)
		//	builder.AddRanges(range.data());

		// handled by custom chars
		//for (const auto& charitem : chars)
		//	builder.AddChar(charitem);
		builder.BuildRanges(&ranges);   // Build the final result (ordered ranges with all the unique characters submitted)
		
		//newFont.ranges = ranges;
		//m_fonts.push_back(newFont);

		//m_dirty = true;
		auto item = mvApp::GetApp()->getItemRegistry().getItem(MV_ATLAS_UUID);
		if (item)
			static_cast<mvStaticTexture*>(item)->markDirty();

		mvToolManager::GetFontManager().m_dirty = true;
	}

	void mvFont::handleSpecificRequiredArgs(PyObject* dict)
	{
		if (!mvApp::GetApp()->getParsers()[s_command].verifyRequiredArguments(dict))
			return;

		for (int i = 0; i < PyTuple_Size(dict); i++)
		{
			PyObject* item = PyTuple_GetItem(dict, i);
			switch (i)
			{
			case 0:
				m_file = ToString(item);
				break;

			case 1:
				m_size = ToFloat(item);
				break;

			default:
				break;
			}
		}
	}

	void mvFont::handleSpecificKeywordArgs(PyObject* dict)
	{
		if (dict == nullptr)
			return;

		if (PyObject* item = PyDict_GetItemString(dict, "default_font")) m_default = ToBool(item);

	}

	void mvFont::getSpecificConfiguration(PyObject* dict)
	{
		if (dict == nullptr)
			return;

		PyDict_SetItemString(dict, "default_font", ToPyBool(m_default));
	}

}