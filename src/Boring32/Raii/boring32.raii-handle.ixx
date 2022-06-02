export module boring32.raii:handle;

export namespace Boring32::Raii
{
	template<typename HANDLE_TYPE>
	class Handle
	{
		public:
			virtual Handle& operator=(const HANDLE_TYPE handle)
			{
				m_handle = handle;
			}

		protected:
			HANDLE_TYPE m_handle;
	};
}