#pragma once
#include "SDK.hpp"
#include "Registory.hpp"

namespace asio
{
	class Interface
	{
		IASIO *iasio;			//!< インターフェースへのポインタ
		void *systemHandle;		//!< システムハンドル
		SubKey subkey;			//!< レジストリの場所

		std::string driverName;	//!< ドライバ名
		long driverVersion;		//!< ドライバのバージョン

	private:
		void RetryCreateInstance(const CLSID& clsid, const SubKey& subkey)
		{
			// デフォルトだとThreadingModelがSTAなので，STA/MTA（Both）に変更して再試行する
			if (Registory::ChangeTheadingModel(subkey) != ERROR_SUCCESS)
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");

			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");
		}

	public:
		/**
		* @params[in] clsid ロードしたいCLSID
		* @params[in] subkey レジストリの位置など
		*/
		Interface(const CLSID& clsid, const SubKey& subkey)
			: subkey(subkey)
		{
			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				RetryCreateInstance(clsid, subkey);

			try
			{
				iasio->init(systemHandle);
			}
			catch (...)
			{
				throw CantHandlingASIODriver("ドライバのハンドルの初期化に失敗しました");
			}

			// 名前とドライバのバージョンだけ取得
			char buffer[360];
			iasio->getDriverName(buffer);
			driverName = buffer;
			driverVersion = iasio->getDriverVersion();
		}

		/**
		* ドライバ名を返す
		*/
		const std::string& Name() const { return driverName; }

		/**
		* ドライバのバージョンを返す
		*/
		const long& Version() const { return driverVersion; }

		/**
		* インターフェースを返す
		*/
		IASIO* IASIO() { return iasio; }
	};
}