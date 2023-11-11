#pragma once

#include <Codes/MouseCodes.h>
#include "Event.h"

#include <sstream>

namespace KDE 
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y) {}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_MouseX, m_MouseY;
	};

	class MouseRawEvent : public Event
	{
	public:
		MouseRawEvent(int xDelta, int yDelta)
			: m_DeltaX(xDelta), m_DeltaY(yDelta) {}

		int GetDeltaX() const { return m_DeltaX; }
		int GetDeltaY() const { return m_DeltaY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseRawEvent: " << GetDeltaX() << ", " << GetDeltaY();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseRaw)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		int m_DeltaX, m_DeltaY;
	};

	class MouseLeaveEvent : public Event
	{
		public:
			MouseLeaveEvent() {}

			std::string ToString() const override
			{
				return "MouseLeaveEvent";
			}

			EVENT_CLASS_TYPE(MouseLeave)
			EVENT_CLASS_CATEGORY(EventCategoryMouse)
	};

	class MouseEnterEvent : public Event
	{
		public:
			MouseEnterEvent() {}

			std::string ToString() const override
			{
				return "MouseEnterEvent";
			}

			EVENT_CLASS_TYPE(MouseEnter)
			EVENT_CLASS_CATEGORY(EventCategoryMouse)
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(int xDelta, int yDelta)
			: m_DeltaX(xDelta), m_DeltaY(yDelta) {}

		int GetDeltaX() const { return m_DeltaX; }
		int GetDeltaY() const { return m_DeltaY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetDeltaX() << ", " << GetDeltaY();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		int m_DeltaX, m_DeltaY;
	};

	class MouseButtonEvent : public Event
	{
	public:
		int GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouseButton | EventCategoryInput)
	protected:
		MouseButtonEvent(int button)
			: m_Button(button) {}

		MouseCode m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}