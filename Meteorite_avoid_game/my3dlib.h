#include <windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <tchar.h>
#include <dinput.h>

struct Model
{
	LPD3DXMESH          pmesh;        // ���b�V��
	D3DMATERIAL9*       pmaterials;   // �}�e���A���̔z��
	LPDIRECT3DTEXTURE9* ptextures;    // �e�N�X�`���̔z��
	DWORD               nummaterials; // �}�e���A���̐�
	BOOL                used;         // �f�[�^�������Ă��邩�����t���O
};

// �O���[�o���ϐ�
extern LPDIRECT3D9       g_pD3D;
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
extern float             g_aspect;
extern Model             g_models[];

// �֐��v���g�^�C�v�錾
HRESULT InitD3DWindow(LPCTSTR wintitle, int w ,int h);
int LoadModel(LPCTSTR filename);
void RenderModel(int idx);
const char *GetKeyState();
void setTimer(int idx, DWORD time);
BOOL isTimerGoal(int idx);
DWORD getPassedTime(int idx);