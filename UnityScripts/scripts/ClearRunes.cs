﻿using UnityEngine;
using System.Collections;

public class ClearRunes : MonoBehaviour {

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

	void OnClick()
	{
		UWCharacter playerUW= GameObject.Find ("Gronk").GetComponent<UWCharacter>();
		if (playerUW!=null)
		{
			playerUW.ActiveRunes[0]=-1;
			playerUW.ActiveRunes[1]=-1;
			playerUW.ActiveRunes[2]=-1;
		}
	}
}
