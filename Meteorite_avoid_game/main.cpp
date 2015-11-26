#include "my3dlib.h"

struct Enemy 
{
	float x, z; // ���݂̍��W
	BOOL used;  // �g�p���������t���O
};

int hjikimodel, hbackmodel, hinsekimodel;
float mx = 0.0f, mz = -2.0f; // ���L�����̍��W
DWORD lasttime;             // �O��̃��[�v�I������(�~���b)
float looptime = 0;         // 1���[�v�ɂ����鎞��(�b)
float speed = 5.0f;         // 5.0m/s
float angle = 0;
float anglesp = 90;         // 90�x/s
const int MAXENEMY = 50;
Enemy enemys[MAXENEMY];

// ���f���̃��[�h
HRESULT LoadModels()
{
	hjikimodel = LoadModel(_T("catsenkan.x"));
	if (hjikimodel == -1) return E_FAIL;
	hbackmodel = LoadModel(_T("back01.x"));
	if (hbackmodel == -1) return E_FAIL;
	hinsekimodel = LoadModel(_T("inseki.x"));
	if (hinsekimodel == -1) return E_FAIL;
	return S_OK;
}

// �����E�r���[�E�ˉe�ϊ��̐ݒ�
void SetViews()
{
	// �����̐ݒ�
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);
	// �r���[�ϊ�
	D3DXVECTOR3 vEyePt(0.0f, 13.0f, -5.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, -1.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);
	// �ˉe�ϊ�
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/4, g_aspect, 1.0f, 100.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
	// �G�z��̏�����
	ZeroMemory(&enemys, sizeof(Enemy)*MAXENEMY);
	// �^�C�}�[�n��
	setTimer(0, 3000);
}

// 覐΂̐ݒ�
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
	if (keys != NULL)
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

	
	// ���L�����̕\��
	// ���[���h���W
	D3DXMATRIXA16 matWorld1, matWorld2;
	D3DXMatrixTranslation(&matWorld1, mx, 0.0f, mz);
	D3DXMatrixRotationY(&matWorld2, r);
	matWorld2 *= matWorld1;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(hjikimodel);
	// �w�i�̕\��
	D3DXMatrixTranslation(&matWorld1, 0.0f, -1.0f, 0.0f);
	D3DXMatrixScaling( &matWorld2, 1.3f, 1.0f, 1.2f);
	matWorld2 *= matWorld1;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(hbackmodel);
	// 覐΂̏o��
	if (isTimerGoal(0) == TRUE)
	{
		AddComet();
		setTimer(0, 2000);
	}
	// 覐΂̕\��,�ړ�
	for (int i = 0; i < MAXENEMY; i = i + 1)
	{
		if (enemys[i].used == TRUE)
		{
			enemys[i].z = enemys[i].z - 2.5f * looptime;   // �ړ�
			if (enemys[i].z < -7.5) enemys[i].used = FALSE; // ��ʊO�ɏo�������
			D3DXMatrixTranslation(&matWorld1, enemys[i].x, 0.0f, enemys[i].z);
			D3DXMatrixRotationY(&matWorld2, (float)timeGetTime()/1000);
			matWorld2 *= matWorld1;
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld2);
			RenderModel(hinsekimodel);
			if ((pow(enemys[i].x - mx, 2)+pow(enemys[i].z-mz, 2)) < 1)
			{
				// �Փ˂��Ă���
				enemys[i].used = FALSE;
			}
		}
	}
}

void Render()
{
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		GameMain();
		g_pd3dDevice->EndScene();
	}
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

// WinMain�֐�
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	srand(timeGetTime());
	if (SUCCEEDED(InitD3DWindow(_T("覐΃Q�[��"), 640, 480)))
	{
		if (FAILED(LoadModels())) return 0; // ���f���̃��[�h
		SetViews();                         // �����E�r���[�E�ˉe�ϊ��̐ݒ�
		// ���b�Z�[�W���[�v
		lasttime = timeGetTime();           // ���[�v�J�n���O�̎��Ԃ��v��
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