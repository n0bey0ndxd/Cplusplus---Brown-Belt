#include "ini.h"

namespace Ini {
	Section& Document::AddSection(std::string name) {
		return sections[name];
	}

	const Section& Document::GetSection(const std::string& name) const {
		return sections.at(name);
	}

	size_t Document::SectionCount() const {
		return sections.size();
	}

	Document Load(std::istream& input) {
		Document doc;
		std::string str;
		Ini::Section* current_section = nullptr;
		while (getline(input, str)) {
			if (str.empty()) {
				continue;
			}

			if (str[0u] == '[') {
				std::string_view str_v(str);
				str_v.remove_suffix(1u);
				str_v.remove_prefix(1u);
				if (str_v.size() != 0u) {
					current_section = &doc.AddSection(static_cast<std::string>(str_v));
				}
			}
			else {
				std::string_view str_v(str);
				size_t pos = str_v.find('=');
				current_section->insert({ static_cast<std::string>(str_v.substr(0, pos)), static_cast<std::string>(str_v.substr(pos + 1)) });
			}
		}

		return doc;
	}
}