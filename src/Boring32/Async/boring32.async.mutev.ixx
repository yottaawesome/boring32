export module boring32.async:mutev;
import :event;

export namespace Boring32::Async
{
	class Mutev
	{
		public:
			enum class Active : unsigned
			{
				None = 0,
				First = 1,
				Second = 2
			};

		public:
			virtual ~Mutev();
			Mutev();
			Mutev(const Active currentActive);

			Mutev(const Mutev& other);
			virtual Mutev& operator=(const Mutev& other);

			Mutev(Mutev&& other) noexcept;
			virtual Mutev& operator=(Mutev&& other) noexcept;

		public:
			virtual void Close();
			virtual void Set(const Active currentActive);
			virtual Active GetCurrentActive() const noexcept;
			virtual Active Switch();

		protected:
			virtual Mutev& Copy(const Mutev& other);
			virtual Mutev& Move(Mutev& other) noexcept;

		protected:
			Event m_first;
			Event m_second;
			Active m_currentActive;
	};
}
