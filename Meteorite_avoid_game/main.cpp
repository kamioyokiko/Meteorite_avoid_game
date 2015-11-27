#include "my3dlib.h"

struct Enemy 
{
	float x, z; // 現在の座標
	BOOL used;  // 使用中を示すフラグ
};

int hjikimodel, hbackmodel, hinsekimodel, hbakumodel;
float mx = 0.0f, mz = -2.0f; // 自キャラの座標
DWORD lasttime;             // 前回のループ終了時間(ミリ秒)
float looptime = 0;         // 1ループにかかる時間(秒)
float speed = 5.0f;         // 5.0m/s
float angle = 0;
float anglesp = 90;         // 90度/s
const int MAXENEMY = 50;
Enemy enemys[MAXENEMY];

enum { GM_MAIN, GM_OVER }; // ゲームモード
int gamemode = GM_MAIN;

int hgofont;               // フォントハンドル

// 関数プロトタイプ宣言
void SetViews(void);
void GameMain(void);

// ゲームオーバーの処理
void GameOver()
{
	GameMain();
	if (getPassedTime(1) < 2000)
	{
		float pt = (float)getPassedTime(1);
		// アルファブレンディング
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); // アルファブレンディングを有効にしている
		g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
		g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		float a = 1.0f - pt / 2000;
		g_pd3dDevice->SetRenderState(D3DRS_BLENDFACTOR, D3DCOLOR_COLORVALUE(a,a,a,a));

		// 自キャラの生成
		D3DXMATRIXA16 matWorld1, matWorld2, matWorld3;
		D3DXMatrixTranslation(&matWorld1, mx, 0.0f, mz);
		D3DXMatrixRotationY(&matWorld2, D3DXToRadian(angle));
		
		D3DXMatrixScaling(&matWorld3, 1.0f+pt/5000.0f, 1.0f+pt/5000.0f, 1.0f+pt/5000.0f);
		matWorld3 = matWorld3 * matWorld2 * matWorld1;
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld3);
		RenderModel(hjikimodel);
		// 爆風の表示
		D3DXMatrixScaling(&matWorld3, 1.0f + pt / 100.0f, 1.0f + pt / 100.0f, 1.0f + pt / 100.0f);
		D3DXMatrixRotationY(&matWorld2, (float)timeGetTime()/1000);
		matWorld3 = matWorld3 * matWorld2 * matWorld1;
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld3);
		RenderModel(hbakumodel);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); // アルファブレンディングを無効にしている
	}

	if (getPassedTime(1) > 3000)
	{
		RECT rc = {1, 161, 641, 261};
		g_ptextsprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
		g_pxfonts[hgofont]->DrawText(g_ptextsprite, _T("GAME OVER"), -1, &rc, DT_CENTER|DT_CENTER, D3DCOLOR_COLORVALUE(0, 1.0f, 1.0f, 1.0f));
		SetRect(&rc, 0, 160, 640, 260);
		g_pxfonts[hgofont]->DrawText(g_ptextsprite, _T("GAME OVER"), -1, &rc, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0, 0, 1.0f));
		g_ptextsprite->End();
	}

	if (getPassedTime(1) > 15000)
	{
		gamemode = GM_MAIN;
		mx = 0.0f;
		mz = -2.0f;
		angle = 0;
		SetViews();
	}
}

// モデルのロード
HRESULT LoadModels()
{
	hjikimodel = LoadModel(_T("catsenkan.x"));
	if (hjikimodel == -1) return E_FAIL;
	hbackmodel = LoadModel(_T("back01.x"));
	if (hbackmodel == -1) return E_FAIL;
	hinsekimodel = LoadModel(_T("inseki.x"));
	if (hinsekimodel == -1) return E_FAIL;
	hbakumodel = LoadModel(_T("bakuha.x"));
	if (hbakumodel == -1) return E_FAIL;

	hgofont = CreateGameFont(_T("ARIAL"), 60, FW_BOLD);
	if (hgofont == -1) return E_FAIL;
	g_pxfonts[hgofont]->PreloadText(_T("GAMEOVER"), 7); // GAMEOVERという文字をプリロード
	g_pxfonts[hgofont]->PreloadCharacters(_T('A'), _T('Z')); // 文字コードの「A」から「Z」までをプリロード

	return S_OK;
}

// 環境光・ビュー・射影変換の設定
void SetViews()
{
	// 環境光の設定
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);
	// ビュー変換
	D3DXVECTOR3 vEyePt(0.0f, 13.0f, -5.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, -1.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
	// 射影変換
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/4, g_aspect, 1.0f, 100.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
	// 敵配列の初期化
	ZeroMemory(&enemys, sizeof(Enemy)*MAXENEMY);
	// タイマー始動
	setTimer(0, 3000);
}

// 隕石の設定
void AddComet()
{
	for (int i = 0; i < MAXENEMY; i = i + 1)
	{
		if (enemys[i].used == FALSE)
		{
			enemys[i].x = (float)(rand()%29)/2 - 7;
			enemys[i].z = 7;
			enemys[i].used = TRUE;
			break;
		}
	}
}

void GameMain()
{
	const char *keys = GetKeyState();
	float v = 0;
	if (keys != NULL && gamemode == GM_MAIN)
	{
		if (keys[DIK_UP] & 0x80)  v = speed * looptime;
		if (keys[DIK_DOWN]&0x80)  v = - speed * looptime;
		if (keys[DIK_LEFT]&0x80)  angle = angle - anglesp * looptime;
		if (keys[DIK_RIGHT]&0x80) angle = angle + anglesp * looptime;
		if (angle < 0) angle = angle + 360;
		if (angle > 360) angle = angle - 360;
	}
	float r = D3DXToRadian(angle);
	mx = mx + v *sinf(r);
	mz = mz + v *cosf(r);
	if (mx < -7) mx = -7;
	if (mx > 7)  mx = 7;
	if (mz < -6) mz = -6;
	if (mz > 6)  mz = 6;

	
	// 自キャラの表示
	// ワールド座標
	D3DXMATRIXA16 matWorld1, matWorld2;
	if (gamemode == GM_MAIN)
	{
		D3DXMatrixTranslation(&matWorld1, mx, 0.0f, mz);
		D3DXMatrixRotationY(&matWorld2, r);
		matWorld2 *= matWorld1;
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
		RenderModel(hjikimodel);
	}
	// 背景の表示
	D3DXMatrixTranslation(&matWorld1, 0.0f, -1.0f, 0.0f);
	D3DXMatrixScaling( &matWorld2, 1.3f, 1.0f, 1.2f);
	matWorld2 *= matWorld1;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(hbackmodel);
	// 隕石の出現
	if (isTimerGoal(0) == TRUE)
	{
		AddComet();
		setTimer(0, 2000);
	}
	// 隕石の表示,移動
	for (int i = 0; i < MAXENEMY; i = i + 1)
	{
		if (enemys[i].used == TRUE)
		{
			enemys[i].z = enemys[i].z - 2.5f * looptime;   // 移動
			if (enemys[i].z < -7.5) enemys[i].used = FALSE; // 画面外に出たら消滅
			D3DXMatrixTranslation(&matWorld1, enemys[i].x, 0.0f, enemys[i].z);
			D3DXMatrixRotationY(&matWorld2, (float)timeGetTime()/1000);
			matWorld2 *= matWorld1;
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
			RenderModel(hinsekimodel);
			if ((pow(enemys[i].x - mx, 2)+pow(enemys[i].z-mz, 2)) < 1 && (gamemode == GM_MAIN))
			{
				// 衝突している
				gamemode = GM_OVER;
				setTimer(1, 0);
			}
		}
	}
}

void Render()
{
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		switch(gamemode)
		{
			case GM_MAIN:
				GameMain();
				break;
			case GM_OVER:
				GameOver();
				break;
		}
		g_pd3dDevice->EndScene();
	}
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

// WinMain関数
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	srand(timeGetTime());
	if (SUCCEEDED(InitD3DWindow(_T("隕石ゲーム"), 640, 480)))
	{
		if (FAILED(LoadModels())) return 0; // モデルのロード
		SetViews();                         // 環境光・ビュー・射影変換の設定
		// メッセージループ
		lasttime = timeGetTime();           // ループ開始直前の時間を計測
		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				Render();
				DWORD curtime = timeGetTime();
				looptime = (float)(curtime - lasttime) / 1000.0f;
				lasttime = curtime;
			}
		}
	}

	UnregisterClass(_T("D3D Window Class"), GetModuleHandle(NULL));
	return 0;
}