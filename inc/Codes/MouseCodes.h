#pragma once

namespace KDE
{
	using MouseCode = int;

	namespace Mouse
	{
		enum : MouseCode
		{
            LeftMouseBtn			=0x01, //Left mouse button
            RightMouseBtn			=0x02, //Right mouse button
            CtrlBrkPrcs				=0x03, //Control-break processing
            MidMouseBtn				=0x04, //Middle mouse button

            ThumbForward			=0x05, //Thumb button back on mouse aka X1
            ThumbBack				=0x06, //Thumb button forward on mouse aka X2
		};
	}
}