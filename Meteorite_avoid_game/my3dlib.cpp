#include "my3dlib.h"

// �O���[�o���ϐ�
LPDIRECT3D9          g_pD3D       = NULL;
LPDIRECT3DDEVICE9    g_pd3dDevice = NULL;
float                g_aspect     = 1.0f;
const int            MAXMODEL     = 64;
Model                g_models[MAXMODEL];
LPDIRECTINPUT8       g_pDI        = NULL;
LPDIRECTINPUTDEVICE8 g_pDIDevice  = NULL;
char                 g_keys[256];
const int            MAXTIMER     = 16;
DWORD                g_goaltimes[MAXTIMER];
const int            MAXFONT = 16;
LPD3DXFONT           g_pxfonts[MAXFONT];
LPD3DXSPRITE         g_ptextsprite = NULL;


int CreateGameFont(LPCTSTR fontname, int height, UINT weight)
{
	// �󂢂Ă���v�f��T��
	int idx;
	for (idx = 0; idx < MAXFONT; idx=idx+1)
	{
		if (g_pxfonts[idx] == NULL) break;
	}
	if (idx >= MAXFONT) return -1;

	// �t�H���g���쐬����
	HRESULT hr = D3DXCreateFont(g_pd3dDevice, -height, 0, weight, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontname, &g_pxfonts[idx]);
	if (FAILED(hr)) return -1;

	return idx;
}

const char *GetKeyState()
{
	HRESULT hr = g_pDIDevice->Acquire();
	if ((hr==DI_OK)||(hr==S_FALSE))
	{
		g_pDIDevice->GetDeviceState(sizeof(g_keys), &g_keys);
		return g_keys;
	}
	return NULL;
}

// ���\�[�X�̉��
void CleanupD3D()
{
	for (int i = 0; i < MAXFONT; i = i + 1)
	{
		if (g_pxfonts[i]) g_pxfonts[i]->Release();
	}
	if (g_ptextsprite) g_ptextsprite->Release();

	for (int i = 0; i < MAXMODEL; i = i + 1)
	{
		if (g_models[i].used == TRUE)
		{
			if (g_models[i].pmaterials != NULL)
			{
				delete[] g_models[i].pmaterials;
			}
			if (g_models[i].ptextures != NULL)
			{
				for (DWORD j = 0; j < g_models[i].nummaterials; j = j + 1)
				{
					g_models[i].ptextures[j]->Release();
				}
				delete[] g_models[i].ptextures;
			}
			if (g_models[i].pmesh != NULL)
			{
				g_models[i].pmesh->Release();
			}
		}
	}
	if (g_pd3dDevice != NULL) g_pd3dDevice->Release();
	if (g_pD3D != NULL) g_pD3D->Release();

	if (g_pDIDevice != NULL)
	{
		g_pDIDevice->Unacquire();
		g_pDIDevice->Release();
	}
	if (g_pDI != NULL) g_pDI->Release();
}

// �E�B���h�E�v���V�[�W��
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
			CleanupD3D();
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// X�t�@�C����ǂݍ��ފ֐�
int LoadModel(LPCTSTR filename)
{
	// ���g�p�̗v�f�iused��FALSE�j��T��
	int idx;
	for (idx = 0; idx < MAXMODEL; idx = idx + 1)
	{
		if (g_models[idx].used == FALSE) break;
	}
	if (idx >= MAXMODEL) return -1;
	LPD3DXBUFFER pD3DXMtrlBuffer; // �ꎞ�L���p�o�b�t�@
								  // X�t�@�C���̓ǂݍ���
	if (FAILED(D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM, g_pd3dDevice, NULL, &pD3DXMtrlBuffer, NULL, &g_models[idx].nummaterials, &g_models[idx].pmesh)))
	{
		MessageBox(NULL, _T("X�t�@�C����������܂���"), _T("3D Lib"), MB_OK);
		return -1;
	}

	// �}�e���A���ƃe�N�X�`���L�^�p�z��̊m��
	D3DXMATERIAL * d3dxMatrials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	int num = g_models[idx].nummaterials;
	g_models[idx].pmaterials = new D3DMATERIAL9[num];
	if (g_models[idx].pmaterials == NULL) return -1;
	g_models[idx].ptextures = new LPDIRECT3DTEXTURE9[num];
	if (g_models[idx].ptextures == NULL) return -1;

	for (int i = 0; i < num; i = i + 1)
	{
		// �}�e���A���̃R�s�[
		g_models[idx].pmaterials[i] = d3dxMatrials[i].MatD3D;
		// �A���r�G���g�F�̐ݒ�
		g_models[idx].pmaterials[i].Ambient = g_models[idx].pmaterials[i].Diffuse;
		// �e�N�X�`���̓ǂݍ���
		g_models[idx].ptextures[i] = NULL;
		if (d3dxMatrials[i].pTextureFilename != NULL && lstrlenA(d3dxMatrials[i].pTextureFilename) > 0)
		{
			if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice, d3dxMatrials[i].pTextureFilename, &g_models[idx].ptextures[i])))
			{
				MessageBox(NULL, _T("�e�N�X�`����������܂���"), _T("3D Lib"), MB_OK);
				return -1;
			}
		}
	}
	pD3DXMtrlBuffer->Release();
	g_models[idx].used = TRUE;
	return idx;
}

void RenderModel(int idx)
{
	if (g_models[idx].used == FALSE) return;

	for (DWORD i = 0; i < g_models[idx].nummaterials; i = i + 1)
	{
		g_pd3dDevice->SetMaterial(&g_models[idx].pmaterials[i]);
		g_pd3dDevice->SetTexture(0, g_models[idx].ptextures[i]);
		g_models[idx].pmesh->DrawSubset(i);
	}
}

void setTimer(int idx, DWORD time)
{
	if (idx > MAXTIMER) return; // �z��̗v�f���𒴂��Ȃ��悤�ɐ���
	g_goaltimes[idx] = timeGetTime() + time;
}

BOOL isTimerGoal(int idx)
{
	if (g_goaltimes[idx] > timeGetTime()) return FALSE;
	return TRUE;
}

DWORD getPassedTime(int idx)
{
	return timeGetTime() - g_goaltimes[idx];
}

// �E�B���h�E���[�h��D3D������
HRESULT InitD3DWindow(LPCTSTR wintitle, int w, int h) {
	ZeroMemory(&g_models, sizeof(Model)*MAXMODEL);
	ZeroMemory(&g_pxfonts, sizeof(LPD3DXFONT)*MAXFONT);
	// �E�B���h�E�N���X�쐬
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("D3D Window Class"), NULL};
	RegisterClassEx(&wc);

	// �E�B���h�E�쐬
	HWND hWnd = CreateWindow( _T("D3D Window Class"), wintitle, WS_OVERLAPPED | WS_SYSMENU, 100, 100, w, h, NULL, NULL, wc.hInstance, NULL);

	// �r���[�|�[�g�A�X�y�N�g�̋L�^
	g_aspect = (float)w / (float)h;

	// D3D9�̍쐬
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// D3D�f�o�C�X�̍쐬(�\�Ȃ�ΌŒ蒸�_�@�\�p�C�v���C�����g�p)
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.Windowed               = TRUE;
	d3dpp.BackBufferFormat       = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) 
	{
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
		{
			return E_FAIL;
		}
	}

	// Z�o�b�t�@���I��
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	ShowWindow(hWnd, SW_SHOWDEFAULT);

	// DirectInput�̏�����
	if (FAILED(DirectInput8Create(wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDI, NULL)))
		return E_FAIL;
	if (FAILED(g_pDI->CreateDevice(GUID_SysKeyboard, &g_pDIDevice, NULL)))
		return E_FAIL;
	g_pDIDevice->SetDataFormat(&c_dfDIKeyboard);
	g_pDIDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);

	// �e�L�X�g�X�v���C�g�̍쐬
	if (FAILED(D3DXCreateSprite(g_pd3dDevice, &g_ptextsprite)))
		return E_FAIL;

	return S_OK;
}