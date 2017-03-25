#ifndef TM_TYPES_H
#define TM_TYPES_H

#include <cstdint>


namespace treemaker {

	enum Flags {
		UNDEFINED = 15,
		AUTOMATIC = 0,
		ASSISTED = 1,
		MANUAL = 2,
		
		DISTRIBUTED = 0,
		FIXED = 1
	};
}

#endif /* TM_TYPES_H */

