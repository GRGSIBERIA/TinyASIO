#pragma once
#include <Windows.h>
#include <exception>
#include <string>
#include <memory>

#include "Registory.hpp"
#include "Interface.hpp"
#include "Structure.hpp"
#include "Channel.hpp"
#include "BufferManager.hpp"

namespace asio
{
	/**
	* ドライバのインスタンスに生成失敗すると呼ばれる
	*/
	class CantCreateInstance : public std::exception
	{
	public:
		CantCreateInstance(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* 二回以上初期化されたりなどで呼び出される
	*/
	class OverTwiceCallException : public std::exception
	{
	public:
		OverTwiceCallException(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* ドライバのハンドルを取得できなかった
	*/
	class CantHandlingASIODriver : public std::exception
	{
	public:
		CantHandlingASIODriver(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* ASIOドライバのインターフェースのラッパクラス
	*/
	class Driver
	{
	private:
		static std::shared_ptr<Driver> driver;	// シングルトン変数

		IASIO *iasio;			// インターフェースへのポインタ
		void *systemHandle;		// 謎のシステムハンドル

		std::string driverName;
		long driverVersion;
		SubKey subkey;

		std::shared_ptr<ChannelManager> channelManager;
		std::shared_ptr<BufferManager> bufferManager;

	private:
		ASIOCallbacks InitNullCallbacks()
		{
			ASIOCallbacks callback;
			callback.asioMessage = NULL;
			callback.bufferSwitch = NULL;
			callback.bufferSwitchTimeInfo = NULL;
			callback.sampleRateDidChange = NULL;
			return callback;
		}


		void RetryCreateInstance(const CLSID& clsid, const SubKey& subkey)
		{
			// デフォルトだとThreadingModelがSTAなので，STA/MTA（Both）に変更して再試行する
			if (Registory::ChangeTheadingModel(subkey) != ERROR_SUCCESS)
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");

			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");
		}


		/**
		* @params[in] clsid ロードしたいCLSID
		* @params[in] subkey レジストリの位置など
		*/
		Driver(const CLSID& clsid, const SubKey& subkey)
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

			channelManager = std::shared_ptr<ChannelManager>(new ChannelManager(iasio));
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(iasio));
		}

	public:

		/**
		* ドライバ名を返す
		*/
		const std::string& Name() const { return driverName; }

		/**
		* ドライバのバージョンを返す
		*/
		const long& Version() const { return driverVersion; }

		/**
		* ドライバのインターフェースを返す
		*/
		const IASIO& Interface() const { return *iasio; }


	public:		// チャンネル周り

		/**
		* 入力チャンネルの配列を返す
		* @return 入力チャンネルの配列
		*/
		inline const std::vector<InputChannel>& InputChannels() const { return channelManager->Inputs(); }
		
		/**
		* 出力チャンネルの配列を返す
		* @return 出力チャンネルの配列
		*/
		inline const std::vector<OutputChannel>& OutputChannels() const { return channelManager->Outputs(); }

		/**
		* チャンネルを追加
		*/
		inline const void AddChannel(const Channel& channel) { bufferManager->AddChannel(channel); }


		/**
		* チャンネルの配列を追加する
		* @params[in] channels チャンネルの配列
		* @params[in] isActiveChannelOnly このフラグが立っていると，有効なチャンネルのみ登録する
		* @tparam CHANNEL InputChannelもしくはOutputChannel
		*/
		template <typename CHANNEL>
		void AddChannels(const std::vector<CHANNEL>& channels, const bool isActiveChannelOnly = true)
		{
			for (const auto& channel : channels)
			{
				if (isActiveChannelOnly)	// isActiveOnlyフラグが立っていれば，isActiveのチェックを行う
				{
					if (channel.isActive)
						bufferManager->AddChannel(channel);
				}
				else
				{
					// 特にフラグが立ってなければ，普通に通す
					bufferManager->AddChannel(channel);
				}
			}
		}


		/**
		* 登録したチャンネルを削除
		*/
		inline const void ClearChannels() { bufferManager->ClearChannel(); }


	public:		// バッファ周り

		/**
		* ASIOのバッファの設定を取得
		* @return バッファの現在の設定
		* @note BufferPreferenceの値を変更してCreateBufferへ渡す
		* @note 必ずしも信用できる値を取得できるとは限らない
		*/
		BufferPreference GetBufferPreference() const
		{
			BufferPreference buf;
			ErrorCheck(iasio->getBufferSize(&buf.minSize, &buf.maxSize, &buf.preferredSize, &buf.granularity));
			return buf;
		}

		/**
		* バッファの生成
		* @params[in] sample サンプリング方法
		* @params[in] bufferPref バッファの設定
		* @return バッファのコントローラ
		*/
		const BufferController& CreateBuffer(const pack::Sample& sample, const BufferPreference& bufferPref)
		{
			ASIOCallbacks callback = callback::CallbackManager::CreateCallbacks();

			//if (bufferManager != nullptr)		// バッファを重複して利用させない作戦
			//	delete bufferManager;
			//bufferManager = new BufferManager(iasio);

			auto& bufferCtrl = bufferManager->CreateBuffer(bufferPref, sample.ToSampleType(), &callback);

			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(iasio));
			
			return bufferCtrl;
		}

		/**
		* バッファを生成する
		* @params[in] sample サンプリング方法
		* @return バッファのコントローラ
		* @note この関数を使うとドライバ側で設定されているバッファサイズを利用します
		*/
		const BufferController& CreateBuffer(const pack::Sample& sample)
		{
			return CreateBuffer(sample, GetBufferPreference());
		}


		/**
		* 存在している全てのチャンネルからバッファを生成する
		* @params[in] sample サンプリング方法
		* @params[in] activeChannelOnly 有効なチャンネルのみ生成する
		* @return バッファのコントローラ
		*/
		const BufferController& CreateBufferAll(const pack::Sample& sample, const bool activeChannelOnly = false)
		{
			bufferManager->ClearChannel();	// 事前にクリアしておく

			AddChannels(channelManager->Inputs(), activeChannelOnly);
			AddChannels(channelManager->Outputs(), activeChannelOnly);

			if (bufferManager->BufferingChannels().size() <= 0)
				throw DontEntryAnyChannels("一つもチャンネルが登録されていません");

			return CreateBuffer(sample);
		}


	public:
		/**
		* ドライバの初期化
		* @params[in] subkey ASIOドライバのSubkey
		* @return subkeyで初期化されたドライバのインスタンス
		* @note 以前に生成されたドライバは破棄される
		*/
		static Driver& Init(const SubKey& subkey)
		{
			auto clsid = Registory::GetCLSID(subkey.registoryPath);
			Driver::driver.reset(new Driver(clsid, subkey), [](Driver *p) { delete p; });
			return *Driver::driver;
		}


		/**
		* ドライバの初期化
		* @params[in] asioDriverName ASIOドライバの名前を検索して，それで初期化を行う
		* @return asioDriverNameで初期化されたドライバのインスタンス
		* @note 見つからない場合はたぶん落ちる
		*/
		static Driver& Init(const std::string& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}
		

		/**
		* ドライバの初期化
		* @params[in] asioDriverName ASIOドライバの名前を検索して，それで初期化を行う
		* @return asioDriverNameで初期化されたドライバのインスタンス
		* @note 見つからない場合はたぶん落ちる
		*/
		static Driver& Init(const std::wstring& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}


		/**
		* ドライバの解放など
		*/
		virtual ~Driver()
		{
			iasio->Release();
		}
	};

	std::shared_ptr<Driver> Driver::driver;
}