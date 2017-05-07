/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

TinyASIO is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

TinyASIO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TinyASIO.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

#pragma once
#include "SDK.hpp"
#include "Registry.hpp"

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
		void RetryCreateInstance(const CLSID& clsid, const SubKey& subkeyData)
		{
			// デフォルトだとThreadingModelがSTAなので，STA/MTA（Both）に変更して再試行する
			if (Registry::ChangeTheadingModel(subkeyData) != ERROR_SUCCESS)
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");

			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");
		}

	public:
		/**
		* @param clsid ロードしたいCLSID
		* @param subkey レジストリの位置など
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
		* @return ドライバ名
		*/
		const std::string& Name() const { return driverName; }

		/**
		* ドライバのバージョンを返す
		* @return ドライバのバージョン
		*/
		const long& Version() const { return driverVersion; }

		/**
		* インターフェースを返す
		* @return ドライバのCOMインターフェース
		*/
		IASIO* IASIO() { return iasio; }

		/**
		* インターフェースを解放
		*/
		void Release() { iasio->Release(); }
	};
}